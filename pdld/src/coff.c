/******************************************************************************
 * @file            coff.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ld.h"
#include "bytearray.h"
#include "coff.h"
#include "xmalloc.h"

#include "coff_bytearray.h"

/* Specification does not seem to say that there must actually be 16 IMAGE_DATA_DIRECTORYs
 * but certain PE/COFF tool does not accept the file if they are not there
 * and it is easier to generate them all even if they are unused. */
#define NUMBER_OF_DATA_DIRECTORIES 16

/* "__imp_" added in front of the "_" prefix means
 * that the symbol is imported directly from Import Address Table
 * instead of from stub generated in .text. */
#define IMP_PREFIX_STR "__imp_"
#define IMP_PREFIX_LEN 6

static int insert_timestamp = 1;
static int kill_at = 0;
static int generate_reloc_section = 1;
static int can_be_relocated = 0;
static int nx_compat = 1;

static int convert_to_flat = 0;

static address_type user_specified_base_address = 0;

static unsigned short wanted_Machine = 0;

static unsigned long SectionAlignment = DEFAULT_SECTION_ALIGNMENT;
static unsigned long FileAlignment = DEFAULT_FILE_ALIGNMENT;
static unsigned short MajorSubsystemVersion = 4;
static unsigned short MinorSubsystemVersion = 0;
static unsigned short Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;

static long size_of_headers;

static address_type size_of_code = 0;
static address_type size_of_initialized_data = 0;
static address_type size_of_uninitialized_data = 0;
static address_type base_of_code = 0;
static address_type base_of_data = 0;

static struct section_part *iat_first_part = NULL;
static struct section_part *iat_last_part = NULL;

static struct section *last_section;

static char *current_import_dll_name = NULL;

struct name_list {
    struct name_list *next;
    int info;
    char *name;
};

enum export_type {
    EXPORT_TYPE_CODE,
    EXPORT_TYPE_DATA,
    EXPORT_TYPE_CONST
};

struct export_name {
    char *name;
    char *name_no_at;
    enum export_type export_type;
};

static struct name_list *export_name_list = NULL;
static struct name_list **last_export_name_list_p = &export_name_list;

static unsigned long translate_section_flags_to_Characteristics (flag_int flags) {

    unsigned long Characteristics = 0;

    if (!(flags & SECTION_FLAG_READONLY)) {
        Characteristics |= IMAGE_SCN_MEM_WRITE;
    }

    if (flags & SECTION_FLAG_CODE) {
        Characteristics |= IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE;
    }

    if (flags & SECTION_FLAG_DATA) {
        Characteristics |= IMAGE_SCN_CNT_INITIALIZED_DATA;
    }

    if (flags & SECTION_FLAG_NEVER_LOAD) {
        Characteristics |= IMAGE_SCN_TYPE_NOLOAD;
    }

    if (flags & SECTION_FLAG_DEBUGGING) {
        Characteristics |= IMAGE_SCN_LNK_INFO;
    }

    if (flags & SECTION_FLAG_EXCLUDE) {
        Characteristics |= IMAGE_SCN_LNK_REMOVE;
    }

    if (!(flags & SECTION_FLAG_NOREAD)) {
        Characteristics |= IMAGE_SCN_MEM_READ;
    }

    if (flags & SECTION_FLAG_SHARED) {
        Characteristics |= IMAGE_SCN_MEM_SHARED;
    }

    /* .bss */
    if ((flags & SECTION_FLAG_ALLOC) && !(flags & SECTION_FLAG_LOAD)) {
        Characteristics |= IMAGE_SCN_CNT_UNINITIALIZED_DATA;
    }

    return Characteristics;
}

static flag_int translate_Characteristics_to_section_flags (unsigned long Characteristics) {

    flag_int flags = 0;

    if (!(Characteristics & IMAGE_SCN_MEM_WRITE)) flags |= SECTION_FLAG_READONLY;

    if (Characteristics & (IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE)) flags |= SECTION_FLAG_CODE;

    if (Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) flags |= SECTION_FLAG_DATA;

    if (Characteristics & IMAGE_SCN_TYPE_NOLOAD) flags |= SECTION_FLAG_NEVER_LOAD;
    
    if (Characteristics & IMAGE_SCN_LNK_INFO) flags |= SECTION_FLAG_DEBUGGING;
    
    if (Characteristics & IMAGE_SCN_LNK_REMOVE) flags |= SECTION_FLAG_EXCLUDE;

    if (!(Characteristics & IMAGE_SCN_MEM_READ)) flags |= SECTION_FLAG_NOREAD;

    if (Characteristics & IMAGE_SCN_MEM_SHARED) flags |= SECTION_FLAG_SHARED;

    if (Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) flags |= SECTION_FLAG_ALLOC;

    return flags;
}

static void write_sections (unsigned char *file)
{
    unsigned char *pos;
    struct section *section;

    pos = file + size_of_headers;

    for (section = all_sections; section; section = section->next) {
        
        struct section_table_entry_internal *hdr = xmalloc (sizeof (*hdr));

        section->object_dependent_data = hdr;

        /* Names should be just truncated as no string table exists. */
        strncpy (hdr->Name, section->name, sizeof (hdr->Name));

        hdr->VirtualSize = section->total_size;
        hdr->VirtualAddress = section->rva;

        if (!section->is_bss) {
            hdr->SizeOfRawData = ALIGN (section->total_size, FileAlignment);
            hdr->PointerToRawData = pos - file;

            section_write (section, pos);
            pos += ALIGN (section->total_size, FileAlignment);
        } else {
            hdr->SizeOfRawData = 0;
            hdr->PointerToRawData = 0;
        }
        hdr->PointerToRelocations = 0;
        hdr->PointerToLinenumbers = 0;

        hdr->NumberOfRelocations = 0;
        hdr->NumberOfLinenumbers = 0;

        hdr->Characteristics = translate_section_flags_to_Characteristics (section->flags);

        if (hdr->Characteristics & IMAGE_SCN_CNT_CODE) {
            if (!base_of_code) base_of_code = hdr->VirtualAddress;
            size_of_code += hdr->VirtualSize;
        } else if (hdr->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) {
            if (!base_of_data) base_of_data = hdr->VirtualAddress;
            size_of_initialized_data += hdr->VirtualSize;
        } else if (hdr->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) {
            size_of_uninitialized_data += hdr->VirtualSize;
        }

        last_section = section;
    }
}

address_type coff_get_base_address (void)
{
    if (user_specified_base_address) return user_specified_base_address;
    
    if (ld_state->create_shared_library) return DEFAULT_DLL_IMAGE_BASE;
    return DEFAULT_EXE_IMAGE_BASE;
}

address_type coff_get_first_section_rva (void)
{
    return ALIGN (size_of_headers, SectionAlignment);
}

static char *unprefix_name (const char *orig_name)
{
    char *name;
    
    if (orig_name[0] == '_') {
        name = xmalloc (strlen (orig_name));
        strcpy (name, orig_name + 1);
    } else {
        name = xstrdup (orig_name);
    }

    return name;
}

static char *unat_name (const char *orig_name)
{
    char *name;
    const char *at_p;
    
    at_p = strchr (orig_name, '@');
    if (at_p) {
        name = xmalloc (at_p - orig_name + 1);
        memcpy (name, orig_name, at_p - orig_name);
        name[at_p - orig_name] = '\0';
    } else {
        name = xstrdup (orig_name);
    }

    return name;
}

static char *undecorate_name (const char *orig_name)
{
    char *name;
    const char *at_p;

    if (orig_name[0] == '_') orig_name++;
    
    at_p = strchr (orig_name, '@');
    if (at_p) {
        name = xmalloc (at_p - orig_name + 1);
        memcpy (name, orig_name, at_p - orig_name);
        name[at_p - orig_name] = '\0';
    } else {
        name = xstrdup (orig_name);
    }

    return name;
}

static int export_name_compar (const void *a, const void *b)
{
    return strcmp (((struct export_name *)a)->name, ((struct export_name *)b)->name);
}

static void write_archive_member_header (unsigned char *pos, const char *Name, unsigned long Size, unsigned long lu_timestamp)
{
    struct IMAGE_ARCHIVE_MEMBER_HEADER_internal member_hdr;

    memset (&member_hdr, ' ', sizeof (member_hdr));
    
    member_hdr.Name[sprintf (member_hdr.Name, Name)] = ' ';
    member_hdr.Date[sprintf (member_hdr.Date, "%lu", lu_timestamp)] = ' ';
    member_hdr.UserID[0] = '0';
    member_hdr.GroupID[0] = '0';
    member_hdr.Mode[0] = '0';
    member_hdr.Size[sprintf (member_hdr.Size, "%lu", Size)] = ' ';
    memcpy (member_hdr.EndOfHeader, IMAGE_ARCHIVE_MEMBER_HEADER_END_OF_HEADER, sizeof (member_hdr.EndOfHeader));
    
    write_struct_IMAGE_ARCHIVE_MEMBER_HEADER (pos, &member_hdr);
}

static void write_implib (struct export_name *export_names, size_t num_names, unsigned long OrdinalBase)
{
    const char *filename;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    unsigned long lu_timestamp;

    unsigned long linker_member_size;
    unsigned long num_linker_member_offsets;
    unsigned char *offset_pos;
    unsigned char *string_table_pos;
    size_t i;

    filename = ld_state->output_implib_filename;

    if (insert_timestamp) {
        time_t timestamp;
        timestamp = time (NULL);
        lu_timestamp = (unsigned long)timestamp;
    } else lu_timestamp = 0;

    file_size = strlen (IMAGE_ARCHIVE_START);
    linker_member_size = 1 * 4;

    num_linker_member_offsets = 0;
    for (i = 0; i < num_names; i++) {
        file_size += sizeof (struct IMAGE_ARCHIVE_MEMBER_HEADER_file);
        file_size += sizeof (struct IMPORT_OBJECT_HEADER_file);
        file_size += 1 + strlen (export_names[i].name) + 1;
        file_size += strlen (ld_state->output_filename) + 1;

        if (export_names[i].export_type == EXPORT_TYPE_CODE) {
            linker_member_size += 4 + 1 + strlen (export_names[i].name) + 1;
            num_linker_member_offsets++;
        }
        linker_member_size += 4 + IMP_PREFIX_LEN + 1 + strlen (export_names[i].name) + 1;
        num_linker_member_offsets++;

        file_size = ALIGN (file_size, 2);
    }

    linker_member_size = ALIGN (linker_member_size, 2);
    file_size += sizeof (struct IMAGE_ARCHIVE_MEMBER_HEADER_file) + linker_member_size;

    file = xmalloc (file_size);
    memset (file, 0, file_size);

    pos = file;

    memcpy (pos, IMAGE_ARCHIVE_START, strlen (IMAGE_ARCHIVE_START));
    pos += strlen (IMAGE_ARCHIVE_START);

    write_archive_member_header (pos, IMAGE_ARCHIVE_LINKER_MEMBER_Name, linker_member_size, lu_timestamp);
    pos += sizeof (struct IMAGE_ARCHIVE_MEMBER_HEADER_file);

    bytearray_write_4_bytes (pos, num_linker_member_offsets, BIG_ENDIAN);
    pos += 4;

    offset_pos = pos;
    string_table_pos = pos + num_linker_member_offsets * 4;
    pos = file + strlen (IMAGE_ARCHIVE_START) + sizeof (struct IMAGE_ARCHIVE_MEMBER_HEADER_file) + linker_member_size;

    for (i = 0; i < num_names; i++) {

        if (export_names[i].export_type == EXPORT_TYPE_CODE) {
            bytearray_write_4_bytes (offset_pos, pos - file, BIG_ENDIAN);
            offset_pos += 4;
            
            string_table_pos++[0] = '_';
            strcpy ((char *)string_table_pos, export_names[i].name);
            string_table_pos += strlen (export_names[i].name) + 1;
        }

        {
            bytearray_write_4_bytes (offset_pos, pos - file, BIG_ENDIAN);
            offset_pos += 4;
            
            memcpy (string_table_pos, IMP_PREFIX_STR, IMP_PREFIX_LEN);
            string_table_pos += IMP_PREFIX_LEN;
            string_table_pos++[0] = '_';
            strcpy ((char *)string_table_pos, export_names[i].name);
            string_table_pos += strlen (export_names[i].name) + 1;
        }

        write_archive_member_header (pos, "IMPORT/",
                                     sizeof (struct IMPORT_OBJECT_HEADER_file)
                                     + 1 + strlen (export_names[i].name) + 1
                                     + strlen (ld_state->output_filename) + 1,
                                     lu_timestamp);
        pos += sizeof (struct IMAGE_ARCHIVE_MEMBER_HEADER_file);

        {
            struct IMPORT_OBJECT_HEADER_internal import_hdr;

            import_hdr.Magic1 = IMAGE_FILE_MACHINE_UNKNOWN;
            import_hdr.Magic2 = IMPORT_OBJECT_HDR_MAGIC2;
            import_hdr.Version = 0;
            import_hdr.Machine = IMAGE_FILE_MACHINE_I386;

            import_hdr.TimeDateStamp = lu_timestamp;
            
            import_hdr.SizeOfData = 1 + strlen (export_names[i].name) + 1 + strlen (ld_state->output_filename) + 1;
            import_hdr.OrdinalHint = OrdinalBase + i;
            
            switch (export_names[i].export_type) {
                case EXPORT_TYPE_CODE: import_hdr.Type = IMPORT_CODE; break;
                case EXPORT_TYPE_DATA: import_hdr.Type = IMPORT_DATA; break;
                case EXPORT_TYPE_CONST: import_hdr.Type = IMPORT_CONST; break;
            }
            
            import_hdr.Type |= (kill_at ? IMPORT_NAME_UNDECORATE : IMPORT_NAME_NOPREFIX) << 2;

            write_struct_IMPORT_OBJECT_HEADER (pos, &import_hdr);
            pos += sizeof (struct IMPORT_OBJECT_HEADER_file);

            pos++[0] = '_';
            strcpy ((char *)pos, export_names[i].name);
            pos += strlen (export_names[i].name) + 1;
            strcpy ((char *)pos, ld_state->output_filename);
            pos += strlen (ld_state->output_filename) + 1;
        }

        pos = file + ALIGN (pos - file, 2);
    }

    {
        FILE *outfile;
        
        if (!(outfile = fopen (filename, "wb"))) {
            ld_error ("cannot open '%s' for writing", filename);
            return;
        }

        if (fwrite (file, file_size, 1, outfile) != 1) {
            ld_error ("writing '%s' file failed", filename);
        }

        fclose (outfile);
    }

    free (file);
    return;
}

#undef KILL_AT_PICK_NAME_I

static void generate_edata (void)
{
    struct object_file *of;
    struct section *section;
    struct section_part *part;
    size_t num_names, name_table_size;
    struct export_name *export_names;
    size_t i;
    struct symbol *symbol;
    struct relocation_entry_internal *relocs;
    size_t name_table_offset;
    struct IMAGE_EXPORT_DIRECTORY_internal ied;
    unsigned long OrdinalBase;

    OrdinalBase = 1;

    {
        struct name_list *name_list, *next_name_list;

        for (name_list = export_name_list, num_names = 0, name_table_size = 0;
             name_list;
             name_list = name_list->next) {
            num_names++;
        }

        export_names = xmalloc (sizeof (*export_names) * num_names);
        for (name_list = export_name_list, i = 0;
             name_list;
             name_list = next_name_list, i++) {
            next_name_list = name_list->next;
            
            export_names[i].name = name_list->name;
            if (kill_at) {
                export_names[i].name_no_at = unat_name (export_names[i].name);
                name_table_size += strlen (export_names[i].name_no_at) + 1;
            } else {
                export_names[i].name_no_at = NULL;
                name_table_size += strlen (export_names[i].name) + 1;
            }
                
            export_names[i].export_type = name_list->info ? EXPORT_TYPE_DATA : EXPORT_TYPE_CODE;
            
            free (name_list);
        }
    }

    qsort (export_names, num_names, sizeof (*export_names), &export_name_compar);

    if (ld_state->output_implib_filename) write_implib (export_names, num_names, OrdinalBase);
    
    name_table_size += strlen (ld_state->output_filename) + 1;

    of = object_file_make (1 + num_names, FAKE_LD_FILENAME);
    section = section_find_or_make (".edata");
    section->section_alignment = SectionAlignment;
    section->flags = translate_Characteristics_to_section_flags (IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ);
    part = section_part_new (section, of);

    part->content_size = (sizeof (struct IMAGE_EXPORT_DIRECTORY_file)
                          + (num_names
                             * (sizeof (struct EXPORT_Address_Table_file)
                                + sizeof (struct EXPORT_Name_Pointer_Table_file)
                                + sizeof (struct EXPORT_Ordinal_Table_file)))
                          + name_table_size);
    part->content = xmalloc (part->content_size);
    memset (part->content, 0, part->content_size);
    section_append_section_part (section, part);

    part->relocation_count = 4 + num_names * 2;
    part->relocation_array = xmalloc (sizeof (struct relocation_entry_internal) * part->relocation_count);
    relocs = part->relocation_array;

    of->symbol_array[0].name = xstrdup (section->name);
    of->symbol_array[0].value = 0;
    of->symbol_array[0].part = part;
    of->symbol_array[0].section_number = 1;

    name_table_offset = (sizeof (struct IMAGE_EXPORT_DIRECTORY_file)
                          + (num_names
                             * (sizeof (struct EXPORT_Address_Table_file)
                                + sizeof (struct EXPORT_Name_Pointer_Table_file)
                                + sizeof (struct EXPORT_Ordinal_Table_file))));

    {
        ied.ExportFlags = 0;
        if (insert_timestamp) {
            /* This timestamp is going to have different value than the timestamp in the COFF header
             * but that is not forbidden. */
            time_t timestamp;
            timestamp = time (NULL);
            ied.TimeDateStamp = (unsigned long)timestamp;
        } else {
            ied.TimeDateStamp = 0;
        }
        ied.MajorVersion = 0;
        ied.MinorVersion = 0;
        ied.NameRVA = name_table_offset;
        ied.OrdinalBase = OrdinalBase;
        ied.AddressTableEntries = num_names;
        ied.NumberOfNamePointers = num_names;
        ied.ExportAddressTableRVA = sizeof (struct IMAGE_EXPORT_DIRECTORY_file);
        ied.NamePointerRVA = ied.ExportAddressTableRVA + num_names * sizeof (struct EXPORT_Address_Table_file);
        ied.OrdinalTableRVA = ied.NamePointerRVA + num_names * sizeof (struct EXPORT_Name_Pointer_Table_file);

        write_struct_IMAGE_EXPORT_DIRECTORY (part->content, &ied);

        for (i = 0; i < 4; i++) {
            relocs[i].SymbolTableIndex = 0;
            relocs[i].Type = IMAGE_REL_I386_DIR32NB;
        }
        relocs[0].VirtualAddress = offsetof (struct IMAGE_EXPORT_DIRECTORY_file, NameRVA);
        relocs[1].VirtualAddress = offsetof (struct IMAGE_EXPORT_DIRECTORY_file, ExportAddressTableRVA);
        relocs[2].VirtualAddress = offsetof (struct IMAGE_EXPORT_DIRECTORY_file, NamePointerRVA);
        relocs[3].VirtualAddress = offsetof (struct IMAGE_EXPORT_DIRECTORY_file, OrdinalTableRVA);
        relocs += 4;
    }

    symbol = of->symbol_array + 1;
    strcpy ((char *)(part->content + name_table_offset), ld_state->output_filename);
    name_table_offset += strlen (ld_state->output_filename) + 1;
    for (i = 0; i < num_names; i++) {
        {
            /* This underscore problem is likely much more complex
             * but this workaround should be enough for now. */
            symbol->name = xmalloc (1 + strlen (export_names[i].name) + 1);
            symbol->name[0] = '_';
            strcpy (symbol->name + 1, export_names[i].name);
            symbol->value = 0;
            symbol->part = NULL;
            symbol->section_number = 0;
            symbol_record_external_symbol (symbol);
            symbol++;
            relocs[0].VirtualAddress = ied.ExportAddressTableRVA + sizeof (struct EXPORT_Address_Table_file) * i;
            relocs[0].SymbolTableIndex = i + 1;
            relocs[0].Type = IMAGE_REL_I386_DIR32NB;
            relocs++;
        }
        {
            struct EXPORT_Name_Pointer_Table_internal npt;
            npt.FunctionNameRVA = name_table_offset;
            write_struct_EXPORT_Name_Pointer_Table (part->content + ied.NamePointerRVA
                                                    + sizeof (struct EXPORT_Name_Pointer_Table_file) * i,
                                                    &npt);
            relocs[0].VirtualAddress = ied.NamePointerRVA + sizeof (struct EXPORT_Name_Pointer_Table_file) * i;
            relocs[0].SymbolTableIndex = 0;
            relocs[0].Type = IMAGE_REL_I386_DIR32NB;
            relocs++;
        }
        {
            struct EXPORT_Ordinal_Table_internal ot;
            ot.FunctionOrdinal = i;
            write_struct_EXPORT_Ordinal_Table (part->content + ied.OrdinalTableRVA
                                               + sizeof (struct EXPORT_Ordinal_Table_file) * i,
                                               &ot);
        }
        {
            char *name;
            name = kill_at ? (export_names[i].name_no_at) : (export_names[i].name);
            strcpy ((char *)(part->content + name_table_offset), name);
            name_table_offset += strlen (name) + 1;
        }
    }

    for (i = 0; i < num_names; i++) {
        free (export_names[i].name);
        free (export_names[i].name_no_at);
    }
    free (export_names);
}

static int check_reloc_section_needed_section_part (struct section_part *part)
{
    struct relocation_entry_internal *relocs;
    size_t i;
    
    relocs = part->relocation_array;
    for (i = 0; i < part->relocation_count; i++) {
        if (wanted_Machine == IMAGE_FILE_MACHINE_AMD64) {
            if (relocs[i].Type == IMAGE_REL_AMD64_ADDR64) return 1;
        } else if (relocs[i].Type == IMAGE_REL_I386_DIR32) return 1;
    }
    
    return 0;
}

static int check_reloc_section_needed (void)
{
    struct section *section;

    for (section = all_sections; section; section = section->next) {
        struct subsection *subsection;
        struct section_part *part;

        for (part = section->first_part; part; part = part->next) {
            if (check_reloc_section_needed_section_part (part)) return 1;
        }

        for (subsection = section->all_subsections; subsection; subsection = subsection->next) {
            for (part = subsection->first_part; part; part = part->next) {
                if (check_reloc_section_needed_section_part (part)) return 1;
            }
        }
    }

    return 0;
}

void coff_before_link (void)
{
    struct section *section;
    struct subsection *subsection;
    struct section_part *part;

    if (export_name_list) {
        generate_edata ();
    }

    /* Certain OS rejects executables with empty .reloc section,
     * so empty .reloc section must NOT be generated. */
    if (!check_reloc_section_needed ()) {
        can_be_relocated = 1;
        generate_reloc_section = 0;
    }

    if (generate_reloc_section) {
        can_be_relocated = 1;
        section = section_find_or_make (".reloc");
        section->section_alignment = SectionAlignment;
        section->flags = translate_Characteristics_to_section_flags (IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE);
    }

    if (wanted_Machine == IMAGE_FILE_MACHINE_AMD64) {
        size_of_headers = (sizeof (struct IMAGE_DOS_HEADER_file) + 4 /* "PE\0\0" */
                           + sizeof (struct coff_header_file)
                           + sizeof (struct optional_header_plus_file)
                           + NUMBER_OF_DATA_DIRECTORIES * sizeof (struct IMAGE_DATA_DIRECTORY_file)
                           + sizeof (struct section_table_entry_file) * section_count ());
    } else {
        size_of_headers = (sizeof (struct IMAGE_DOS_HEADER_file) + 4 /* "PE\0\0" */
                           + sizeof (struct coff_header_file)
                           + sizeof (struct optional_header_file)
                           + NUMBER_OF_DATA_DIRECTORIES * sizeof (struct IMAGE_DATA_DIRECTORY_file)
                           + sizeof (struct section_table_entry_file) * section_count ());
    }
    size_of_headers = ALIGN (size_of_headers, FileAlignment);

    /* .idata$2 contains Import Directory Table which needs to be terminated with null entry. */
    section = section_find (".idata");
    if (section == NULL) return;
    subsection = subsection_find (section, "2");
    if (subsection == NULL) return;
    
    part = section_part_new (section, object_file_make (0, FAKE_LD_FILENAME));

    part->content_size = sizeof (struct IMPORT_Directory_Table_file);
    part->content = xmalloc (part->content_size);
    memset (part->content, 0, part->content_size);

    subsection_append_section_part (subsection, part);

    /* .idata$5 is Import Address Table. */
    subsection = subsection_find (section, "5");
    if (subsection == NULL) return;

    iat_first_part = subsection->first_part;
    if (iat_first_part == NULL) return;
    for (part = iat_first_part; part->next; part = part->next) ;
    iat_last_part = part;
}

static void coff_relocate_part_64 (struct section_part *part)
{
    struct relocation_entry_internal *relocs;
    size_t i;
    
    relocs = part->relocation_array;
    for (i = 0; i < part->relocation_count; i++) {

        struct symbol *symbol;

        if (relocs[i].Type == IMAGE_REL_AMD64_ABSOLUTE) continue;

        symbol = part->of->symbol_array + relocs[i].SymbolTableIndex;
        if (symbol_is_undefined (symbol)) {
            if ((symbol = symbol_find (symbol->name)) == NULL) {
                symbol = part->of->symbol_array + relocs[i].SymbolTableIndex;
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "external symbol '%s' not found in hashtab",
                                             symbol->name);
            }
            if (symbol_is_undefined (symbol)) {
                ld_error ("%s:(%s+0x%lx): undefined reference to '%s'",
                          part->of->filename,
                          part->section->name,
                          relocs[i].VirtualAddress,
                          symbol->name);
                continue;
            }
        }

        switch (relocs[i].Type) {

            case IMAGE_REL_AMD64_ADDR64:
                {
                    unsigned long result;

                    /* It should be actually 8 bytes but 64-bit int is not yet available. */
                    bytearray_read_4_bytes (&result, part->content + relocs[i].VirtualAddress, LITTLE_ENDIAN);

                    result += symbol_get_value_with_base (symbol);

                    bytearray_write_4_bytes (part->content + relocs[i].VirtualAddress, result, LITTLE_ENDIAN);
                }
                break;

            case IMAGE_REL_AMD64_ADDR32NB:
                {
                    unsigned long result;

                    bytearray_read_4_bytes (&result, part->content + relocs[i].VirtualAddress, LITTLE_ENDIAN);

                    result += symbol_get_value_no_base (symbol);

                    bytearray_write_4_bytes (part->content + relocs[i].VirtualAddress, result, LITTLE_ENDIAN);
                }
                break;

            case IMAGE_REL_AMD64_REL32:
                {
                    unsigned long result;

                    bytearray_read_4_bytes (&result, part->content + relocs[i].VirtualAddress, LITTLE_ENDIAN);

                    result += symbol_get_value_no_base (symbol) - (part->rva + relocs[i].VirtualAddress) - 4;

                    bytearray_write_4_bytes (part->content + relocs[i].VirtualAddress, result, LITTLE_ENDIAN);
                }
                break;
            
            case IMAGE_REL_AMD64_ADDR32:
            case IMAGE_REL_AMD64_REL32_1:
            case IMAGE_REL_AMD64_REL32_2:
            case IMAGE_REL_AMD64_REL32_3:
            case IMAGE_REL_AMD64_REL32_4:
            case IMAGE_REL_AMD64_REL32_5:
            case IMAGE_REL_AMD64_SECTION:
            case IMAGE_REL_AMD64_SECREL:
            case IMAGE_REL_AMD64_SECREL7:
            case IMAGE_REL_AMD64_TOKEN:
            case IMAGE_REL_AMD64_SREL32:
            case IMAGE_REL_AMD64_PAIR:
            case IMAGE_REL_AMD64_SSPAN32:
                ld_internal_error_at_source (__FILE__, __LINE__, "+++relocation type 0x%04hx not supported yet", relocs[i].Type);
                break;

            default:
                /* There is no point in continuing, the object is broken. */
                ld_fatal_error ("invalid relocation type 0x%04hx (origin object '%s')", relocs[i].Type, part->of->filename);
                break;

        }
        
    }
}

void coff_relocate_part (struct section_part *part)
{
    struct relocation_entry_internal *relocs;
    size_t i;

    if (wanted_Machine == IMAGE_FILE_MACHINE_AMD64) {
        coff_relocate_part_64 (part);
        return;
    }
    
    relocs = part->relocation_array;
    for (i = 0; i < part->relocation_count; i++) {

        struct symbol *symbol;

        if (relocs[i].Type == IMAGE_REL_I386_ABSOLUTE) continue;

        symbol = part->of->symbol_array + relocs[i].SymbolTableIndex;
        if (symbol_is_undefined (symbol)) {
            if ((symbol = symbol_find (symbol->name)) == NULL) {
                ld_internal_error_at_source (__FILE__, __LINE__, "external symbol not found in hashtab");
            }
            if (symbol_is_undefined (symbol)) {
                ld_error ("%s:(%s+0x%lx): undefined reference to '%s'",
                          part->of->filename,
                          part->section->name,
                          relocs[i].VirtualAddress,
                          symbol->name);
                continue;
            }
        }

        switch (relocs[i].Type) {

            case IMAGE_REL_I386_DIR32:
                {
                    unsigned long result;

                    bytearray_read_4_bytes (&result, part->content + relocs[i].VirtualAddress, LITTLE_ENDIAN);

                    result += symbol_get_value_with_base (symbol);

                    bytearray_write_4_bytes (part->content + relocs[i].VirtualAddress, result, LITTLE_ENDIAN);
                }
                break;

            case IMAGE_REL_I386_DIR32NB:
                {
                    unsigned long result;

                    bytearray_read_4_bytes (&result, part->content + relocs[i].VirtualAddress, LITTLE_ENDIAN);

                    result += symbol_get_value_no_base (symbol);

                    bytearray_write_4_bytes (part->content + relocs[i].VirtualAddress, result, LITTLE_ENDIAN);
                }
                break;

            case IMAGE_REL_I386_REL32:
                {
                    unsigned long result;

                    bytearray_read_4_bytes (&result, part->content + relocs[i].VirtualAddress, LITTLE_ENDIAN);

                    result += symbol_get_value_no_base (symbol) - (part->rva + relocs[i].VirtualAddress) - 4;

                    bytearray_write_4_bytes (part->content + relocs[i].VirtualAddress, result, LITTLE_ENDIAN);
                }
                break;

            case IMAGE_REL_I386_SECTION:
            case IMAGE_REL_I386_SECREL:
            case IMAGE_REL_I386_TOKEN:
            case IMAGE_REL_I386_SECREL7:
                ld_internal_error_at_source (__FILE__, __LINE__, "+++relocation type 0x%04hx not supported yet", relocs[i].Type);
                break;

            case IMAGE_REL_I386_DIR16:
            case IMAGE_REL_I386_REL16:
            case IMAGE_REL_I386_SEG12:
                /* There is no point in continuing, the user is using very outdated objects. */
                ld_fatal_error ("relocation type 0x%04hx is no longer supported according to the specification", relocs[i].Type);
                break;

            default:
                /* There is no point in continuing, the object is broken. */
                ld_fatal_error ("invalid relocation type 0x%04hx (origin object '%s')", relocs[i].Type, part->of->filename);
                break;

        }
        
    }
}

address_type coff_calculate_entry_point (void)
{
    struct symbol *symbol;

    symbol = symbol_find ("_mainCRTStartup");
    if (symbol) return symbol_get_value_no_base (symbol);

    return 0;
}

#define FLOOR_TO(to_floor, floor) ((to_floor) / (floor) * (floor))

static void generate_base_relocation_block (struct section *reloc_section,
                                            struct IMAGE_BASE_RELOCATION_internal *ibr_hdr_p,
                                            unsigned long num_relocs,
                                            struct section *saved_section,
                                            struct section_part *saved_part)
{
    struct section_part *reloc_part;
    unsigned char *write_pos;
    struct section *section;

    /* There must be even number of Base relocation WORDs
     * because the start of the blocks must be aligned on 4 byte boundary. */
    ibr_hdr_p->SizeOfBlock = ALIGN (sizeof (struct IMAGE_BASE_RELOCATION_file) + num_relocs * 2, 4);

    reloc_part = section_part_new (reloc_section, object_file_make (0, FAKE_LD_FILENAME));
    reloc_part->content_size = ibr_hdr_p->SizeOfBlock;
    reloc_part->content = xmalloc (reloc_part->content_size);
    reloc_part->content[reloc_part->content_size - 2] = reloc_part->content[reloc_part->content_size - 1] = 0;

    write_struct_IMAGE_BASE_RELOCATION (reloc_part->content, ibr_hdr_p);
    write_pos = reloc_part->content + sizeof (struct IMAGE_BASE_RELOCATION_file);

    for (section = saved_section; section; section = section->next) {
        struct section_part *part;

        for (part = ((section == saved_section) ? saved_part : section->first_part); part; part = part->next) {
            struct relocation_entry_internal *relocs;
            size_t i;

            relocs = part->relocation_array;
            for (i = 0; i < part->relocation_count; i++) {
                unsigned short base_relocation_type;
                
                if (wanted_Machine == IMAGE_FILE_MACHINE_AMD64) {
                    if (relocs[i].Type != IMAGE_REL_AMD64_ADDR64) continue;
                    base_relocation_type = IMAGE_REL_BASED_DIR64;
                } else {
                    if (relocs[i].Type != IMAGE_REL_I386_DIR32) continue;
                    base_relocation_type = IMAGE_REL_BASED_HIGHLOW;
                }
                
                if (part->rva + relocs[i].VirtualAddress < ibr_hdr_p->RVAOfBlock) continue;

                {
                    unsigned short rel_word;

                    rel_word = (part->rva + relocs[i].VirtualAddress - ibr_hdr_p->RVAOfBlock) & 0xfff;
                    rel_word |= base_relocation_type << 12;

                    bytearray_write_2_bytes (write_pos, rel_word, LITTLE_ENDIAN);
                    write_pos += 2;
                }
                if (!--num_relocs) goto finish;
            }
        }
    }
    
finish:
    section_append_section_part (reloc_section, reloc_part);
    reloc_section->total_size += reloc_part->content_size;
}

void coff_after_link (void)
{
    struct IMAGE_BASE_RELOCATION_internal ibr_hdr;
    unsigned long num_relocs;
    struct section *saved_section;
    struct section_part *saved_part;
    struct section *reloc_section;
    struct section *section;

    if (!generate_reloc_section) return;

    if ((reloc_section = section_find (".reloc")) == NULL) {
        ld_internal_error_at_source (__FILE__, __LINE__, ".reloc section could not be found");
    }

    ibr_hdr.RVAOfBlock = 0;
    num_relocs = 0;
    saved_section = NULL;
    saved_part = NULL;

    for (section = all_sections; section; section = section->next) {
        struct section_part *part;

        for (part = section->first_part; part; part = part->next) {
            struct relocation_entry_internal *relocs;
            size_t i;

            relocs = part->relocation_array;
            for (i = 0; i < part->relocation_count; i++) {
                if (wanted_Machine == IMAGE_FILE_MACHINE_AMD64) {
                    if (relocs[i].Type != IMAGE_REL_AMD64_ADDR64) continue;
                } else {
                    if (relocs[i].Type != IMAGE_REL_I386_DIR32) continue;
                }

                if (num_relocs
                    && part->rva + relocs[i].VirtualAddress >= ibr_hdr.RVAOfBlock + BASE_RELOCATION_PAGE_SIZE) {
                    generate_base_relocation_block (reloc_section,
                                                    &ibr_hdr,
                                                    num_relocs,
                                                    saved_section,
                                                    saved_part);
                    num_relocs = 0;
                }

                if (num_relocs == 0) {
                    ibr_hdr.RVAOfBlock = FLOOR_TO (part->rva + relocs[i].VirtualAddress, BASE_RELOCATION_PAGE_SIZE);
                    saved_section = section;
                    saved_part = part;
                }

                num_relocs++;                    
            }
        }
    }

    if (num_relocs) {
        generate_base_relocation_block (reloc_section,
                                        &ibr_hdr,
                                        num_relocs,
                                        saved_section,
                                        saved_part);
    }
}

void coff_write (const char *filename)
{
    FILE *outfile;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    struct IMAGE_DOS_HEADER_internal dos_hdr;
    struct coff_header_internal coff_hdr;
    struct optional_header_internal optional_hdr;
    struct optional_header_plus_internal optional_hdr_plus;

    struct section *section;

    if (!(outfile = fopen (filename, "wb"))) {
        ld_error ("cannot open '%s' for writing", filename);
        return;
    }

    {
        size_t total_section_size_to_write = 0;

        for (section = all_sections; section; section = section->next) {
            if (!section->is_bss) total_section_size_to_write += ALIGN (section->total_size, FileAlignment);
        }

        file_size = size_of_headers + total_section_size_to_write;
    }

    file = xmalloc (file_size);
    memset (file, 0, file_size);

    write_sections (file);

    pos = file;

    memset (&dos_hdr, 0, sizeof (dos_hdr));

    dos_hdr.Magic[0] = 'M';
    dos_hdr.Magic[1] = 'Z';

    dos_hdr.SizeOfHeaderInParagraphs = sizeof (struct IMAGE_DOS_HEADER_file) / IMAGE_DOS_HEADER_PARAGRAPH_SIZE;

    dos_hdr.OffsetToNewEXEHeader = sizeof (struct IMAGE_DOS_HEADER_file);

    write_struct_IMAGE_DOS_HEADER (pos, &dos_hdr);
    pos += sizeof (struct IMAGE_DOS_HEADER_file);

    memcpy (pos, "PE\0\0", 4);
    pos += 4;

    coff_hdr.Machine = wanted_Machine;
    coff_hdr.NumberOfSections = section_count ();
    
    if (insert_timestamp) {
        /* Specification says TimeDateStamp should be low 32 bits of time_t
         * even though the meaning of time_t is not portable. */
        time_t timestamp;
        timestamp = time (NULL);
        coff_hdr.TimeDateStamp = (unsigned long)timestamp;
    } else {
        coff_hdr.TimeDateStamp = 0;
    }
    
    coff_hdr.PointerToSymbolTable = 0;
    coff_hdr.NumberOfSymbols = 0;
    coff_hdr.SizeOfOptionalHeader = ((wanted_Machine == IMAGE_FILE_MACHINE_AMD64)
                                     ? sizeof (struct optional_header_plus_file)
                                     : sizeof (struct optional_header_file));   
    coff_hdr.SizeOfOptionalHeader += NUMBER_OF_DATA_DIRECTORIES * sizeof (struct IMAGE_DATA_DIRECTORY_file);
    
    coff_hdr.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_DEBUG_STRIPPED;
    if (wanted_Machine == IMAGE_FILE_MACHINE_AMD64) {
        coff_hdr.Characteristics |= IMAGE_FILE_LARGE_ADDRESS_AWARE;
    } else {
        coff_hdr.Characteristics |= IMAGE_FILE_32BIT_MACHINE;
    }
    
    if (ld_state->create_shared_library) coff_hdr.Characteristics |= IMAGE_FILE_DLL;
    if (!can_be_relocated) coff_hdr.Characteristics |= IMAGE_FILE_RELOCS_STRIPPED;

    write_struct_coff_header (pos, &coff_hdr);
    pos += sizeof (struct coff_header_file);

    if (wanted_Machine == IMAGE_FILE_MACHINE_AMD64) {
        memset (&optional_hdr_plus, 0, sizeof (optional_hdr_plus));

        optional_hdr_plus.Magic = PE32_PLUS_MAGIC;
        optional_hdr_plus.MajorLinkerVersion = LD_MAJOR_VERSION;
        optional_hdr_plus.MinorLinkerVersion = LD_MINOR_VERSION;
        
        /* Seems that these 3 fields should be rounded up to FileAlignment. */
        optional_hdr_plus.SizeOfCode = ALIGN (size_of_code, FileAlignment);
        optional_hdr_plus.SizeOfInitializedData = ALIGN (size_of_initialized_data, FileAlignment);
        optional_hdr_plus.SizeOfUninitializedData = ALIGN (size_of_uninitialized_data, FileAlignment);

        optional_hdr_plus.AddressOfEntryPoint = ld_state->entry_point;

        optional_hdr_plus.BaseOfCode = base_of_code;

        optional_hdr_plus.ImageBase = ld_state->base_address;
        
        optional_hdr_plus.SectionAlignment = SectionAlignment;
        optional_hdr_plus.FileAlignment = FileAlignment;

        optional_hdr_plus.MajorOperatingSystemVersion = 4;
        optional_hdr_plus.MajorImageVersion = 1;
        optional_hdr_plus.MajorSubsystemVersion = MajorSubsystemVersion;
        optional_hdr_plus.MinorSubsystemVersion = MinorSubsystemVersion;

        if (last_section) optional_hdr_plus.SizeOfImage = ALIGN (last_section->rva + last_section->total_size, SectionAlignment);
        else optional_hdr_plus.SizeOfImage = ALIGN (size_of_headers, SectionAlignment);
        optional_hdr_plus.SizeOfHeaders = size_of_headers;

        optional_hdr_plus.Subsystem = Subsystem;
        
        optional_hdr_plus.DllCharacteristics = 0;
        if (nx_compat) optional_hdr_plus.DllCharacteristics |= IMAGE_DLLCHARACTERISTICS_NX_COMPAT;
        if (can_be_relocated) optional_hdr_plus.DllCharacteristics |= IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE;

        /* No idea how to determine the following. */
        optional_hdr_plus.SizeOfStackReserve = 0x200000;
        optional_hdr_plus.SizeOfStackCommit = 0x1000;
        optional_hdr_plus.SizeOfHeapReserve = 0x100000;
        optional_hdr_plus.SizeOfHeapCommit = 0x1000;

        optional_hdr_plus.NumberOfRvaAndSizes = NUMBER_OF_DATA_DIRECTORIES;

        write_struct_optional_header_plus (pos, &optional_hdr_plus);
        pos += sizeof (struct optional_header_plus_file);
    } else {
        memset (&optional_hdr, 0, sizeof (optional_hdr));

        optional_hdr.Magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
        optional_hdr.MajorLinkerVersion = LD_MAJOR_VERSION;
        optional_hdr.MinorLinkerVersion = LD_MINOR_VERSION;
        
        /* Seems that these 3 fields should be rounded up to FileAlignment. */
        optional_hdr.SizeOfCode = ALIGN (size_of_code, FileAlignment);
        optional_hdr.SizeOfInitializedData = ALIGN (size_of_initialized_data, FileAlignment);
        optional_hdr.SizeOfUninitializedData = ALIGN (size_of_uninitialized_data, FileAlignment);

        optional_hdr.AddressOfEntryPoint = ld_state->entry_point;

        optional_hdr.BaseOfCode = base_of_code;
        optional_hdr.BaseOfData = base_of_data;

        optional_hdr.ImageBase = ld_state->base_address;
        optional_hdr.SectionAlignment = SectionAlignment;
        optional_hdr.FileAlignment = FileAlignment;

        optional_hdr.MajorOperatingSystemVersion = 4;
        optional_hdr.MajorImageVersion = 1;
        optional_hdr.MajorSubsystemVersion = MajorSubsystemVersion;
        optional_hdr.MinorSubsystemVersion = MinorSubsystemVersion;

        if (last_section) optional_hdr.SizeOfImage = ALIGN (last_section->rva + last_section->total_size, SectionAlignment);
        else optional_hdr.SizeOfImage = ALIGN (size_of_headers, SectionAlignment);
        optional_hdr.SizeOfHeaders = size_of_headers;

        optional_hdr.Subsystem = Subsystem;
        
        optional_hdr.DllCharacteristics = 0;
        if (nx_compat) optional_hdr.DllCharacteristics |= IMAGE_DLLCHARACTERISTICS_NX_COMPAT;
        if (can_be_relocated) optional_hdr.DllCharacteristics |= IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE;

        /* No idea how to determine the following. */
        optional_hdr.SizeOfStackReserve = 0x200000;
        optional_hdr.SizeOfStackCommit = 0x1000;
        optional_hdr.SizeOfHeapReserve = 0x100000;
        optional_hdr.SizeOfHeapCommit = 0x1000;

        optional_hdr.NumberOfRvaAndSizes = NUMBER_OF_DATA_DIRECTORIES;

        write_struct_optional_header (pos, &optional_hdr);
        pos += sizeof (struct optional_header_file);
    }

    {
        int i;
        
        for (i = 0; i < NUMBER_OF_DATA_DIRECTORIES; i++) {
            struct IMAGE_DATA_DIRECTORY_internal idd = {0};

            switch (i) {

                case 0:
                    /* EXPORT Table. */
                    section = section_find (".edata");
                    if (section) {
                        idd.VirtualAddress = section->rva;
                        idd.Size = section->total_size;
                    }
                    break;

                case 1:
                    /* IMPORT Table. */
                    section = section_find (".idata");
                    if (section) {
                        idd.VirtualAddress = section->rva;
                        idd.Size = section->total_size;
                    }
                    break;

                case 5:
                    /* BASE RELOCATION Table. */
                    section = section_find (".reloc");
                    if (section) {
                        idd.VirtualAddress = section->rva;
                        idd.Size = section->total_size;
                    }
                    break;
                
                case 12:
                    /* IMPORT Address Table. */
                    section = section_find (".idata");
                    if (section && iat_first_part) {
                        idd.VirtualAddress = iat_first_part->rva;
                        idd.Size = iat_last_part->rva + iat_last_part->content_size - iat_first_part->rva;
                    }
                    break;

            }
            write_struct_IMAGE_DATA_DIRECTORY (pos, &idd);
            pos += sizeof (struct IMAGE_DATA_DIRECTORY_file);
        }

    }

    for (section = all_sections; section; section = section->next) {
        struct section_table_entry_internal *hdr = section->object_dependent_data;

        write_struct_section_table_entry (pos, hdr);
        pos += sizeof (struct section_table_entry_file);

        free (hdr);
    }

    if (convert_to_flat
        && wanted_Machine == IMAGE_FILE_MACHINE_AMD64) {
        ld_error ("--convert-to-flat is not supported for 64-bit");
        convert_to_flat = 0;
    }

    if (convert_to_flat) {
        file = xrealloc (file, optional_hdr.SizeOfImage);
        memset (file + file_size, '\0', optional_hdr.SizeOfImage - file_size);
        file_size = optional_hdr.SizeOfImage;
        file[0] = 0xE9;
        bytearray_write_4_bytes (file + 1, ld_state->entry_point - 5, LITTLE_ENDIAN);
    }

    if (fwrite (file, file_size, 1, outfile) != 1) {
        ld_error ("writing '%s' file failed", filename);
    }

    free (file);
    fclose (outfile);
    return;
}

#define CHECK_READ(memory_position, size_to_read) \
    do { if (((memory_position) - file + (size_to_read) > file_size) \
             || (memory_position) < file) ld_fatal_error ("corrupted input file"); } while (0)

static void interpret_dot_drectve_section (const unsigned char *file, size_t file_size, const unsigned char *pos, size_t size)
{
    char *temp_buf, *p;

    /* According to specification the content of .drectve should be a string
     * but that cannot be trusted, so NUL is appended. */
    temp_buf = xmalloc (size + 1);
    CHECK_READ (pos, size);
    memcpy (temp_buf, pos, size);
    temp_buf[size] = '\0';
    
    if (pos[0] == 0xEF && pos[1] == 0xBB && pos[2] == 0xBF) {
        ld_internal_error_at_source (__FILE__, __LINE__, "UTF-8 byte order marker not yet supported at the start of .drectve section");
    }

    p = temp_buf;
    while (*p) {
        while (*p == ' ') p++;
        if (strncmp (p, "-export:", 8) == 0) {
            char *q;
            char saved_c;
            int data = 0;
            
            p += 8;
            q = strchr (p, ' ');
            if (q == NULL) q = p + strlen (p);
            saved_c = *q;
            *q = '\0';
            {
                /* There is ",data" added to data symbol names,
                 * so this handles it. */
                char *comma;
                
                comma = strchr (p, ',');
                if (comma) {
                    if (strcmp (comma, ",data") == 0) data = 1;
                    else {
                        ld_internal_error_at_source (__FILE__, __LINE__,
                                                     "unsupported comma argument to option -export: '%s'",
                                                     comma);
                    }
                    *comma = '\0';
                }
            }
            {
                struct name_list *name_list;
                name_list = xmalloc (sizeof (*name_list));
                name_list->name = xstrdup (p);
                name_list->info = data;
                name_list->next = NULL;
                *last_export_name_list_p = name_list;
                last_export_name_list_p = &name_list->next;
            }
            *q = saved_c;
            p = q;
        } else if (*p) {
            ld_internal_error_at_source (__FILE__, __LINE__, "unsupported .drectve option: %s", p);
        }
    }

    free (temp_buf);
}

union sym_tab_entry {
    struct symbol_table_entry_internal sym;
    unsigned char aux[sizeof (struct symbol_table_entry_file)];
};

static union sym_tab_entry *read_symbol_table (unsigned char *file,
                                               size_t file_size,
                                               const char *filename,
                                               const struct coff_header_internal *coff_hdr_p,
                                               unsigned long *comdat_aux_symbol_indexes)
{
    union sym_tab_entry *read_symtab;
    unsigned char *pos;
    unsigned long i;
    unsigned char aux_num = 0;

    pos = file + coff_hdr_p->PointerToSymbolTable;
    CHECK_READ (pos, sizeof (struct symbol_table_entry_file) * coff_hdr_p->NumberOfSymbols);

    read_symtab = xmalloc (sizeof (*read_symtab) * coff_hdr_p->NumberOfSymbols);
    
    memset (comdat_aux_symbol_indexes, 0, sizeof (*comdat_aux_symbol_indexes) * coff_hdr_p->NumberOfSections);

    for (i = 0; i < coff_hdr_p->NumberOfSymbols; i++, pos += sizeof (struct symbol_table_entry_file)) {
        if (aux_num) {
            memcpy (read_symtab[i].aux, pos, sizeof (read_symtab[i].aux));
            aux_num--;
            continue;
        }
        
        read_struct_symbol_table_entry (&read_symtab[i].sym, pos);
        
        aux_num = read_symtab[i].sym.NumberOfAuxSymbols;

        if (read_symtab[i].sym.SectionNumber > 0
            && read_symtab[i].sym.SectionNumber <= coff_hdr_p->NumberOfSections) {
            short sec_num = read_symtab[i].sym.SectionNumber - 1;

            if (!comdat_aux_symbol_indexes[sec_num]) {
                comdat_aux_symbol_indexes[sec_num] = i + 1;
            }
        }
    }
    
    if (aux_num) {
        ld_error ("incorrect NumberOfAuxSymbols, exceeds symbol table size");
        free (read_symtab);
        return NULL;
    }    

    return read_symtab;
}

static void read_coff_object (unsigned char *file, size_t file_size, const char *filename)
{
    struct coff_header_internal coff_hdr;
    struct string_table_header_internal string_table_hdr;
    char *string_table = NULL;
    struct section_table_entry_internal section_hdr;
    union sym_tab_entry *read_symtab = NULL;
    unsigned long *comdat_aux_symbol_indexes;
    struct section_part dummy_comdat_part_s;

    unsigned char *pos;

    struct object_file *of;
    struct section_part **part_p_array;
    struct section_part *bss_part;
    long bss_section_number;
    unsigned long i;

    pos = file;
    CHECK_READ (pos, sizeof (struct coff_header_file));
    read_struct_coff_header (&coff_hdr, pos);

    if (wanted_Machine && wanted_Machine != coff_hdr.Machine) {
        ld_error ("Machine field mismatch between objects");
        return;
    }

    wanted_Machine = coff_hdr.Machine;

    pos = file + coff_hdr.PointerToSymbolTable + sizeof (struct symbol_table_entry_file) * coff_hdr.NumberOfSymbols;
    CHECK_READ (pos, sizeof (struct string_table_header_file));
    read_struct_string_table_header (&string_table_hdr, pos);
    if (string_table_hdr.StringTableSize < 4) ld_error ("invalid string table size: %lu", string_table_hdr.StringTableSize);
    else {
        CHECK_READ (pos, string_table_hdr.StringTableSize);
        string_table = (char *)pos;
    }

    part_p_array = xmalloc (sizeof (*part_p_array) * (coff_hdr.NumberOfSections + 1));
    of = object_file_make (coff_hdr.NumberOfSymbols, filename);
    bss_part = NULL;
    bss_section_number = 0;

    comdat_aux_symbol_indexes = xmalloc (sizeof (*comdat_aux_symbol_indexes) * coff_hdr.NumberOfSections);
    if (coff_hdr.NumberOfSymbols) {
        read_symtab = read_symbol_table (file, file_size, filename, &coff_hdr, comdat_aux_symbol_indexes);
        if (read_symtab == NULL) {
            free (comdat_aux_symbol_indexes);
            return;
        }
    }

    for (i = 0; i < coff_hdr.NumberOfSections; i++) {

        pos = file + sizeof (struct coff_header_file) + sizeof (struct section_table_entry_file) * i;
        CHECK_READ (pos, sizeof (struct section_table_entry_file));
        read_struct_section_table_entry (&section_hdr, pos);

        if ((section_hdr.Characteristics & IMAGE_SCN_LNK_REMOVE) || section_hdr.SizeOfRawData == 0) {
            /* Empty section. */
            part_p_array[i + 1] = NULL;
            continue;
        }

        {
            struct section *section;
            struct subsection *subsection;

            {
                char *section_name;
                char *p;

                section_name = xstrndup (section_hdr.Name, 8);

                if (section_name[0] == '/') {
                    unsigned long offset = 0;
                    
                    offset = strtoul (section_name + 1, NULL, 10);
                    if (offset < string_table_hdr.StringTableSize) {
                        free (section_name);
                        section_name = xstrdup (string_table + offset);
                    } else ld_fatal_error ("invalid offset into string table");
                }

                p = strchr (section_name, '$');
                if (p) {
                    *p = '\0';
                    p++;
                }

                if (section_hdr.Characteristics & IMAGE_SCN_LNK_COMDAT) {
                    struct aux_section_symbol_internal aux_symbol;
                    unsigned long sym_i = comdat_aux_symbol_indexes[i];
                    
                    if (!sym_i) {
                        ld_error ("missing section symbol for COMDAT section '%s'",
                                  section_name);
                        return;
                    }
                    
                    if (read_symtab[sym_i - 1].sym.Value != 0
                        || read_symtab[sym_i - 1].sym.Type != IMAGE_SYM_TYPE_NULL
                        || read_symtab[sym_i - 1].sym.StorageClass != IMAGE_SYM_CLASS_STATIC
                        || read_symtab[sym_i - 1].sym.NumberOfAuxSymbols != 1) {
                        ld_error ("invalid section symbol for COMDAT section '%s'",
                                  section_name);
                        return;
                    }

                    read_struct_aux_section_symbol (&aux_symbol, read_symtab[sym_i].aux);
                    
                    if (aux_symbol.Length != section_hdr.SizeOfRawData
                        || aux_symbol.NumberOfRelocations != section_hdr.NumberOfRelocations
                        || aux_symbol.NumberOfLinenumbers != section_hdr.NumberOfLinenumbers) {
                        ld_warn ("section auxiliary symbol inconsistent with section header for COMDAT section '%s'",
                                 section_name);
                    }

                    if (aux_symbol.Selection != IMAGE_COMDAT_SELECT_ANY) {
                        ld_internal_error_at_source (__FILE__, __LINE__,
                                                     "only IMAGE_COMDAT_SELECT_ANY Selection is supported for COMDAT");
                    }

                    section = section_find (section_name);
                    if (section) {
                        if (p) subsection = subsection_find (section, p);

                        if (!p || subsection) {
                            part_p_array[i + 1] = &dummy_comdat_part_s;
                            free (section_name);
                            continue;
                        }
                    }
                }

                if (strcmp (section_name, ".drectve") == 0) {
                    interpret_dot_drectve_section (file,
                                                   file_size,
                                                   file + section_hdr.PointerToRawData,
                                                   section_hdr.SizeOfRawData);
                    part_p_array[i + 1] = NULL;
                    free (section_name);
                    continue;
                }

                section = section_find_or_make (section_name);

                section->section_alignment = SectionAlignment;
                section->flags = translate_Characteristics_to_section_flags (section_hdr.Characteristics);
                if (section_hdr.PointerToRawData == 0) {
                    section->is_bss = 1;
                    bss_section_number = i;
                }

                if (p) subsection = subsection_find_or_make (section, p);
                else subsection = NULL;

                free (section_name);
            }

            {
                struct section_part *part = section_part_new (section, of);

                part->content_size = section_hdr.SizeOfRawData;
                if (section_hdr.PointerToRawData) {
                    pos = file + section_hdr.PointerToRawData;
                    part->content = xmalloc (part->content_size);

                    CHECK_READ (pos, part->content_size);
                    memcpy (part->content, pos, part->content_size);
                }
                if (section->is_bss) {
                    bss_part = part;
                }

                if (section_hdr.PointerToRelocations && section_hdr.NumberOfRelocations) {

                    size_t j;
                    struct relocation_entry_internal *relocations;

                    if (!section_hdr.PointerToRawData) {
                        ld_fatal_error ("section '%s' is BSS but has relocations", section->name);
                    }

                    pos = file + section_hdr.PointerToRelocations;

                    part->relocation_array = xmalloc (sizeof (struct relocation_entry_internal) * section_hdr.NumberOfRelocations);
                    part->relocation_count = section_hdr.NumberOfRelocations;
                    relocations = part->relocation_array;
                    CHECK_READ (pos, sizeof (struct relocation_entry_file) * section_hdr.NumberOfRelocations);
                    for (j = 0; j < section_hdr.NumberOfRelocations; j++) {
                        read_struct_relocation_entry (relocations + j,
                                                      pos + sizeof (struct relocation_entry_file) * j);
                    }
                }
                
                if (subsection) {
                    subsection_append_section_part (subsection, part);
                } else {
                    section_append_section_part (section, part);
                }

                part_p_array[i + 1] = part;
            }
        }
    }

    for (i = 0; i < coff_hdr.NumberOfSymbols; i++) {

        struct symbol_table_entry_internal *coff_symbol = &read_symtab[i].sym;
        struct symbol *symbol = of->symbol_array + i;

        if (memcmp (coff_symbol->Name, "\0\0\0\0", 4) == 0) {

            unsigned long offset = 0;

            bytearray_read_4_bytes (&offset, (unsigned char *)(coff_symbol->Name + 4), LITTLE_ENDIAN);

            if (offset < string_table_hdr.StringTableSize) {
                symbol->name = xstrdup (string_table + offset);
            } else ld_fatal_error ("invalid offset into string table");
            
        } else symbol->name = xstrndup (coff_symbol->Name, 8);

        if (coff_symbol->SectionNumber > 0
            && coff_symbol->SectionNumber <= coff_hdr.NumberOfSections
            && part_p_array[coff_symbol->SectionNumber] == &dummy_comdat_part_s) {
            /* The COMDAT section was discarded,
             * so all symbols defined there are now undefined. */
            coff_symbol->SectionNumber = IMAGE_SYM_UNDEFINED;
            coff_symbol->Value = 0;
        }
        
        symbol->value = coff_symbol->Value;
        symbol->section_number = coff_symbol->SectionNumber;
        
        if (coff_symbol->SectionNumber == IMAGE_SYM_UNDEFINED) {
            if (symbol->value) {
                /* It is a common symbol. */
                if (bss_part == NULL) {
                    struct section *section;

                    section = section_find_or_make (".bss");

                    section->section_alignment = SectionAlignment;
                    section->flags = translate_Characteristics_to_section_flags (IMAGE_SCN_CNT_UNINITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE);
                    section->is_bss = 1;
                    bss_section_number = coff_hdr.NumberOfSections ? coff_hdr.NumberOfSections : 1;

                    bss_part = section_part_new (section, of);
                    bss_part->content_size = 0;
                    section_append_section_part (section, bss_part);
                }
                
                symbol->part = bss_part;
                bss_part->content_size += symbol->value;
                symbol->value = bss_part->content_size - symbol->value;
                symbol->section_number = bss_section_number;             
            } else {
                symbol->part = NULL;
            }
        } else if (coff_symbol->SectionNumber > 0
                   && coff_symbol->SectionNumber <= coff_hdr.NumberOfSections) {
            symbol->part = part_p_array[coff_symbol->SectionNumber];
        } else if (coff_symbol->SectionNumber == IMAGE_SYM_ABSOLUTE) {
            symbol->section_number = ABSOLUTE_SECTION_NUMBER;
            symbol->part = NULL;
        } else if (coff_symbol->SectionNumber == IMAGE_SYM_DEBUG) {
            symbol->section_number = DEBUG_SECTION_NUMBER;
            symbol->part = NULL;
        } else if (coff_symbol->SectionNumber > coff_hdr.NumberOfSections) {
            ld_error ("invalid symbol SectionNumber: %hi", coff_symbol->SectionNumber);
            symbol->part = NULL;
        } else ld_internal_error_at_source (__FILE__, __LINE__,
                                            "+++not yet supported symbol SectionNumber: %hi",
                                            coff_symbol->SectionNumber);

        if (coff_symbol->StorageClass == IMAGE_SYM_CLASS_EXTERNAL) {
            symbol_record_external_symbol (symbol);
        }

        if (coff_symbol->NumberOfAuxSymbols) {
            for (i++; coff_symbol->NumberOfAuxSymbols; coff_symbol->NumberOfAuxSymbols--) {
                symbol = of->symbol_array + i;
                memset (symbol, 0, sizeof (*symbol));
                symbol->auxiliary = 1;
            }
        }
    }

    free (comdat_aux_symbol_indexes);
    free (read_symtab);
    free (part_p_array);
}

static void import_generate_head (const char *dll_name, const char *filename)
{
    struct object_file *of;
    struct section *section;
    struct subsection *subsection;
    struct section_part *part;

    of = object_file_make (3, filename);
    section = section_find_or_make (".idata");
    section->section_alignment = SectionAlignment;
    section->flags = translate_Characteristics_to_section_flags (IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE);

    subsection = subsection_find_or_make (section, "2");
    part = section_part_new (section, of);
    subsection_append_section_part (subsection, part);

    part->content_size = sizeof (struct IMPORT_Directory_Table_file);
    part->content = xmalloc (part->content_size);

    {
        struct IMPORT_Directory_Table_internal import_dt;
        struct relocation_entry_internal *relocs;
        int i;

        import_dt.ImportNameTableRVA = 4;
        import_dt.TimeDateStamp = 0;
        import_dt.ForwarderChain = 0;
        import_dt.NameRVA = 0;
        import_dt.ImportAddressTableRVA = 4;

        write_struct_IMPORT_Directory_Table (part->content, &import_dt);

        part->relocation_count = 3;
        part->relocation_array = xmalloc (sizeof (struct relocation_entry_internal) * part->relocation_count);
        relocs = part->relocation_array;

        for (i = 0; i < 3; i++) {
            relocs[i].SymbolTableIndex = i;
            relocs[i].Type = IMAGE_REL_I386_DIR32NB;
        }
        relocs[0].VirtualAddress = offsetof (struct IMPORT_Directory_Table_file, ImportNameTableRVA);
        relocs[1].VirtualAddress = offsetof (struct IMPORT_Directory_Table_file, ImportAddressTableRVA);
        relocs[2].VirtualAddress = offsetof (struct IMPORT_Directory_Table_file, NameRVA);
    }
    
    subsection = subsection_find_or_make (section, "4");
    part = section_part_new (section, of);
    subsection_append_section_part (subsection, part);
    part->content_size = 4;
    part->content = xmalloc (part->content_size);
    memset (part->content, 0, part->content_size);

    of->symbol_array[0].name = xstrdup (".idata$4");
    of->symbol_array[0].value = 0;
    of->symbol_array[0].part = part;
    of->symbol_array[0].section_number = 2;

    subsection = subsection_find_or_make (section, "5");
    part = section_part_new (section, of);
    subsection_append_section_part (subsection, part);
    part->content_size = 4;
    part->content = xmalloc (part->content_size);
    memset (part->content, 0, part->content_size);

    of->symbol_array[1].name = xstrdup (".idata$5");
    of->symbol_array[1].value = 0;
    of->symbol_array[1].part = part;
    of->symbol_array[1].section_number = 3;

    subsection = subsection_find_or_make (section, "7");
    part = section_part_new (section, of);
    subsection_append_section_part (subsection, part);
    part->content_size = ALIGN (strlen (dll_name) + 1, 2);
    part->content = xmalloc (part->content_size);
    memset (part->content, 0, part->content_size);
    strcpy ((char *)(part->content), dll_name);

    of->symbol_array[2].name = xstrdup (".idata$7");
    of->symbol_array[2].value = 0;
    of->symbol_array[2].part = part;
    of->symbol_array[2].section_number = 4;
}

static void import_generate_import (const char *import_name,
                                    short OrdinalHint,
                                    short ImportType,
                                    short ImportNameType,
                                    const char *filename)
{
    struct section_part *dot_idata5_part;
    struct object_file *of;
    struct section *section;
    struct subsection *subsection;
    struct section_part *part;
    struct relocation_entry_internal *relocs;
    struct symbol *symbol;

    /* 2 symbols are internal for .idata$5 and .idata$6. */
    of = object_file_make (2 + 1 + ((ImportType == IMPORT_CODE) ? 1 : 0), filename);
    symbol = of->symbol_array;

    section = section_find_or_make (".idata");

    subsection = subsection_find_or_make (section, "4");
    part = section_part_new (section, of);
    subsection_append_section_part (subsection, part);
    part->content_size = 4;
    part->content = xmalloc (part->content_size);
    memset (part->content, 0, part->content_size);
    
    part->relocation_count = 1;
    part->relocation_array = xmalloc (sizeof (struct relocation_entry_internal) * part->relocation_count);
    relocs = part->relocation_array;
    relocs[0].SymbolTableIndex = 1;
    relocs[0].Type = IMAGE_REL_I386_DIR32NB;
    relocs[0].VirtualAddress = 0;

    subsection = subsection_find_or_make (section, "5");
    part = section_part_new (section, of);
    subsection_append_section_part (subsection, part);
    part->content_size = 4;
    part->content = xmalloc (part->content_size);
    memset (part->content, 0, part->content_size);

    symbol->name = xstrdup (".idata$5");
    symbol->value = 0;
    symbol->part = part;
    symbol->section_number = 2;
    symbol++;

    part->relocation_count = 1;
    part->relocation_array = xmalloc (sizeof (struct relocation_entry_internal) * part->relocation_count);
    relocs = part->relocation_array;
    relocs[0].SymbolTableIndex = 1;
    relocs[0].Type = IMAGE_REL_I386_DIR32NB;
    relocs[0].VirtualAddress = 0;

    dot_idata5_part = part;

    subsection = subsection_find_or_make (section, "6");
    part = section_part_new (section, of);
    subsection_append_section_part (subsection, part);

    {
        char *real_import_name;

        switch (ImportNameType) {

            case IMPORT_ORDINAL:
                ld_internal_error_at_source (__FILE__, __LINE__, "IMPORT_ORDINAL is not yet supported");
                break;

            case IMPORT_NAME:
                real_import_name = xstrdup (import_name);
                break;
            
            case IMPORT_NAME_NOPREFIX:
                real_import_name = unprefix_name (import_name);
                break;

            case IMPORT_NAME_UNDECORATE:
                real_import_name = undecorate_name (import_name);
                break;

            default:
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "unsupported ImportNameType: %i",
                                             ImportNameType);
                break;
            
        }
        
        part->content_size = ALIGN (2 + strlen (real_import_name) + 1, 2);
        part->content = xmalloc (part->content_size);
        memset (part->content, 0, part->content_size);
        bytearray_write_2_bytes (part->content, OrdinalHint, LITTLE_ENDIAN);
        strcpy ((char *)(part->content + 2), real_import_name);

        free (real_import_name);
    }

    symbol->name = xstrdup (".idata$6");
    symbol->value = 0;
    symbol->part = part;
    symbol->section_number = 3;
    symbol++;

    if (ImportType == IMPORT_CODE) {
        section = section_find_or_make (".text");
        part = section_part_new (section, of);
        section_append_section_part (section, part);

        part->content_size = 8;
        part->content = xmalloc (part->content_size);
        memcpy (part->content, "\xFF\x25\x00\x00\x00\x00\x90\x90", 8);

        symbol->name = xstrdup (import_name);
        symbol->value = 0;
        symbol->part = part;
        symbol->section_number = 4;
        symbol_record_external_symbol (symbol);
        symbol++;

        part->relocation_count = 1;
        part->relocation_array = xmalloc (sizeof (struct relocation_entry_internal) * part->relocation_count);
        relocs = part->relocation_array;
        relocs[0].SymbolTableIndex = 0;
        relocs[0].Type = IMAGE_REL_I386_DIR32;
        relocs[0].VirtualAddress = 2;
    }
    
    symbol->name = xmalloc (IMP_PREFIX_LEN + strlen (import_name) + 1);
    memcpy (symbol->name, IMP_PREFIX_STR, IMP_PREFIX_LEN);
    strcpy (symbol->name + IMP_PREFIX_LEN, import_name);
    
    symbol->value = 0;
    symbol->part = dot_idata5_part;
    symbol->section_number = 2;
    symbol_record_external_symbol (symbol);
}

static void import_generate_end (void)
{
    struct object_file *of;
    struct section *section;
    struct subsection *subsection;
    struct section_part *part;

    of = object_file_make (0, FAKE_LD_FILENAME);
    section = section_find_or_make (".idata");

    subsection = subsection_find_or_make (section, "4");
    part = section_part_new (section, of);
    subsection_append_section_part (subsection, part);
    part->content_size = 4;
    part->content = xmalloc (part->content_size);
    memset (part->content, 0, part->content_size);

    subsection = subsection_find_or_make (section, "5");
    part = section_part_new (section, of);
    subsection_append_section_part (subsection, part);
    part->content_size = 4;
    part->content = xmalloc (part->content_size);
    memset (part->content, 0, part->content_size);
}

static void read_import_object (unsigned char *file, size_t file_size, const char *filename)
{
    struct IMPORT_OBJECT_HEADER_internal import_hdr;
    char *import_name;
    char *dll_name;
    
    unsigned char *pos;

    pos = file;
    CHECK_READ (pos, sizeof (struct IMPORT_OBJECT_HEADER_file));
    read_struct_IMPORT_OBJECT_HEADER (&import_hdr, pos);
    pos += sizeof (struct IMPORT_OBJECT_HEADER_file);

    if (import_hdr.Machine != IMAGE_FILE_MACHINE_I386) {
        ld_error ("unrecognized Machine in import header");
        return;
    }

    if ((import_hdr.Type & 0x3) == IMPORT_CONST) {
        ld_internal_error_at_source (__FILE__, __LINE__,
                                     "+++not yet supported import header import Type: 0x%x",
                                     import_hdr.Type & 0x3);
    }
        
    CHECK_READ (pos, 2);
    import_name = (char *)pos;
    dll_name = import_name + strlen (import_name) + 1;

    if (current_import_dll_name && strcmp (dll_name, current_import_dll_name)) {
        import_generate_end ();
        free (current_import_dll_name);
        current_import_dll_name = NULL;
    }
    if (current_import_dll_name == NULL) {
        current_import_dll_name = xstrdup (dll_name);
        import_generate_head (current_import_dll_name, filename);
    }
    
    import_generate_import (import_name, import_hdr.OrdinalHint, import_hdr.Type & 0x3, import_hdr.Type >> 2, filename);  
}

static void strip_trailing_spaces (char *str)
{
    char *p = str + strlen (str);

    while (p > str && p[-1] == ' ') p--;

    *p = '\0';
}

struct lm_offset_name_entry {
    unsigned long offset;
    char *name;
};

struct archive_member_header {
    char *name;
    unsigned long size;
};

static void read_archive_member_header (unsigned char *pos, struct archive_member_header *hdr)
{
    struct IMAGE_ARCHIVE_MEMBER_HEADER_internal member_header;
    char *tmp;

    read_struct_IMAGE_ARCHIVE_MEMBER_HEADER (&member_header, pos);

    hdr->name = xstrndup (member_header.Name, sizeof (member_header.Name));
    strip_trailing_spaces (hdr->name);
    tmp = xstrndup (member_header.Size, sizeof (member_header.Size));
    strip_trailing_spaces (tmp);
    hdr->size = strtoul (tmp, NULL, 10);
    free (tmp);
}

static struct lm_offset_name_entry *read_linker_member (unsigned char *file, size_t file_size, unsigned long *NumberOfSymbols_p)
{
    unsigned long NumberOfSymbols;
    struct lm_offset_name_entry *offset_name_table;

    unsigned char *pos;

    unsigned long i;
    unsigned char *string_table_pos;

    pos = file;

    CHECK_READ (pos, 4);
    bytearray_read_4_bytes (&NumberOfSymbols, pos, BIG_ENDIAN);
    pos += 4;

    offset_name_table = xmalloc (sizeof (*offset_name_table) * NumberOfSymbols);
    string_table_pos = pos + NumberOfSymbols * 4;

    for (i = 0; i < NumberOfSymbols; i++) {

        CHECK_READ (pos, 4);
        bytearray_read_4_bytes (&offset_name_table[i].offset, pos, BIG_ENDIAN);
        pos += 4;
        
        CHECK_READ (string_table_pos, 1);
        offset_name_table[i].name = (char *)string_table_pos;
        string_table_pos += strlen ((char *)string_table_pos) + 1;

    }

    *NumberOfSymbols_p = NumberOfSymbols;

    return offset_name_table;
}

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

static int read_coff_archive_member (unsigned char *file, size_t file_size, unsigned char *pos, const char *archive_name)
{
    struct archive_member_header hdr;
    unsigned short Machine;
    int ret;
    char *filename;

    CHECK_READ (pos, sizeof (struct IMAGE_ARCHIVE_MEMBER_HEADER_file));
    read_archive_member_header (pos, &hdr);
    pos += sizeof (struct IMAGE_ARCHIVE_MEMBER_HEADER_file);

    {
        size_t archive_name_len = strlen (archive_name);
        size_t member_name_len = strlen (hdr.name);

        /* Outside of members starting with '/' the '/' serves as name terminator
         * according to the specification. */
        if (member_name_len && hdr.name[member_name_len - 1] == '/' && hdr.name[0] != '/') member_name_len--;
        
        filename = xmalloc (archive_name_len + 1 + member_name_len + 1 + 1);
        memcpy (filename, archive_name, archive_name_len);
        filename[archive_name_len] = '(';
        memcpy (filename + archive_name_len + 1, hdr.name, member_name_len);
        filename[archive_name_len + 1 + member_name_len] = ')';
        filename[archive_name_len + 1 + member_name_len + 1] = '\0';
    }

    CHECK_READ (pos, 2);
    bytearray_read_2_bytes (&Machine, pos, LITTLE_ENDIAN);
    if (Machine == IMAGE_FILE_MACHINE_I386
        || Machine == IMAGE_FILE_MACHINE_AMD64) {
        read_coff_object (pos, MIN (hdr.size, file_size - (pos - file)), filename);
        ret = 1;
    } else if (Machine == IMAGE_FILE_MACHINE_UNKNOWN) {
        unsigned short Magic2;
        
        CHECK_READ (pos + 2, 2);
        bytearray_read_2_bytes (&Magic2, pos + 2, LITTLE_ENDIAN);
        if (Magic2 == IMPORT_OBJECT_HDR_MAGIC2) {
            read_import_object (pos, MIN (hdr.size, file_size - (pos - file)), filename);
        } else goto unrecognized;

        ret = 2;
    } else {
unrecognized:
        ld_error ("%s: unrecognized archive member object format", filename);
        ret = 0;
    }

    free (hdr.name);
    free (filename);

    return ret;
}    

static void read_coff_archive (unsigned char *file, size_t file_size, const char *archive_name)
{
    struct archive_member_header hdr;

    struct lm_offset_name_entry *offset_name_table;
    unsigned long NumberOfSymbols;
    unsigned long i;
    
    unsigned char *pos;

    unsigned long start_header_object_offset = 0;
    unsigned long end_header_object_offset = 0;

    pos = file + strlen (IMAGE_ARCHIVE_START);
    CHECK_READ (pos, sizeof (struct IMAGE_ARCHIVE_MEMBER_HEADER_file));
    read_archive_member_header (pos, &hdr);
    pos += sizeof (struct IMAGE_ARCHIVE_MEMBER_HEADER_file);

    if (strcmp (hdr.name, IMAGE_ARCHIVE_LINKER_MEMBER_Name) == 0) {
        offset_name_table = read_linker_member (pos, MIN (hdr.size, file_size - (pos - file)), &NumberOfSymbols);
    } else {
        offset_name_table = NULL;
        ld_error ("IMAGE_ARCHIVE_LINKER_MEMBER missing from archive");
    }

    free (hdr.name);

    if (offset_name_table == NULL) return;
    
    /* This is necessary because the member containing symbol "__head_something"
     * contains the first part of the .idata content
     * and the member containing symbol "_something_iname" contains the terminators for the .idata content.
     * (Applies only to the traditional import library format,
     * for the short format whole .idata is automatically generated.)*/
    for (i = 0; i < NumberOfSymbols && (!start_header_object_offset || !end_header_object_offset); i++) {
        if (strncmp (offset_name_table[i].name, "__head_", 7) == 0) {
            start_header_object_offset = offset_name_table[i].offset;
        } else if (strlen (offset_name_table[i].name) > 6
                   && strcmp (offset_name_table[i].name + strlen (offset_name_table[i].name) - 6, "_iname") == 0) {
            end_header_object_offset = offset_name_table[i].offset;
        }
    }

    if (start_header_object_offset)
        read_coff_archive_member (file, file_size, file + start_header_object_offset, archive_name);

    while (1) {
        int change = 0;

        for (i = 0; i < NumberOfSymbols; i++) {
            int ret;
            struct symbol *symbol = symbol_find (offset_name_table[i].name);

            if (symbol == NULL) continue;
            if (!symbol_is_undefined (symbol)) continue;

            if (offset_name_table[i].offset == start_header_object_offset
                || offset_name_table[i].offset == end_header_object_offset) continue;
            
            pos = file + offset_name_table[i].offset;
            ret = read_coff_archive_member (file, file_size, pos, archive_name);
            if (ret == 0) return;
            /* If the archive member is a real object (not short import entry),
             * it might require more symbols. */
            if (ret == 1) change = 1;
        }

        if (change == 0) break;
    }

    if (end_header_object_offset)
        read_coff_archive_member (file, file_size, file + end_header_object_offset, archive_name);

    if (current_import_dll_name) {
        import_generate_end ();
        free (current_import_dll_name);
        current_import_dll_name = NULL;
    }

    free (offset_name_table);
}

void coff_read (const char *filename)
{
    unsigned short Machine;

    unsigned char *file;
    size_t file_size;

    if (read_file_into_memory (filename, &file, &file_size)) {
        ld_error ("failed to read file '%s' into memory", filename);
        return;
    }

    CHECK_READ (file, strlen (IMAGE_ARCHIVE_START));

    bytearray_read_2_bytes (&Machine, file, LITTLE_ENDIAN);

    if (Machine == IMAGE_FILE_MACHINE_I386
        || Machine == IMAGE_FILE_MACHINE_AMD64) {
        read_coff_object (file, file_size, filename);
    } else if (memcmp (file, IMAGE_ARCHIVE_START, strlen (IMAGE_ARCHIVE_START)) == 0) {
        read_coff_archive (file, file_size, filename);
    } else ld_error ("unrecognized file format");

    free (file);
}

#include "options.h"

enum option_index {

    COFF_OPTION_IGNORED = 0,
    COFF_OPTION_FILE_ALIGNMENT,
    COFF_OPTION_IMAGE_BASE,
    COFF_OPTION_SECTION_ALIGNMENT,
    COFF_OPTION_SUBSYSTEM,
    COFF_OPTION_INSERT_TIMESTAMP,
    COFF_OPTION_NO_INSERT_TIMESTAMP,
    COFF_OPTION_KILL_AT,
    COFF_OPTION_ENABLE_RELOC_SECTION,
    COFF_OPTION_DISABLE_RELOC_SECTION,
    COFF_OPTION_NX_COMPAT,
    COFF_OPTION_DISABLE_NX_COMPAT,
    COFF_OPTION_CONVERT_TO_FLAT

};

#define STR_AND_LEN(str) (str), (sizeof (str) - 1)
static const struct long_option long_options[] = {
    
    { STR_AND_LEN("file-alignment"), COFF_OPTION_FILE_ALIGNMENT, OPTION_HAS_ARG},
    { STR_AND_LEN("image-base"), COFF_OPTION_IMAGE_BASE, OPTION_HAS_ARG},
    { STR_AND_LEN("section-alignment"), COFF_OPTION_SECTION_ALIGNMENT, OPTION_HAS_ARG},
    { STR_AND_LEN("subsystem"), COFF_OPTION_SUBSYSTEM, OPTION_HAS_ARG},
    { STR_AND_LEN("insert-timestamp"), COFF_OPTION_INSERT_TIMESTAMP, OPTION_NO_ARG},
    { STR_AND_LEN("no-insert-timestamp"), COFF_OPTION_NO_INSERT_TIMESTAMP, OPTION_NO_ARG},
    { STR_AND_LEN("kill-at"), COFF_OPTION_KILL_AT, OPTION_NO_ARG},
    { STR_AND_LEN("enable-reloc-section"), COFF_OPTION_ENABLE_RELOC_SECTION, OPTION_NO_ARG},
    { STR_AND_LEN("disable-reloc-section"), COFF_OPTION_DISABLE_RELOC_SECTION, OPTION_NO_ARG},
    { STR_AND_LEN("nxcompat"), COFF_OPTION_NX_COMPAT, OPTION_NO_ARG},
    { STR_AND_LEN("disable-nxcompat"), COFF_OPTION_DISABLE_NX_COMPAT, OPTION_NO_ARG},
    { STR_AND_LEN("convert-to-flat"), COFF_OPTION_CONVERT_TO_FLAT, OPTION_NO_ARG},
    { NULL, 0, 0}

};
#undef STR_AND_LEN

void coff_print_help (void)
{
    printf ("i386pe:\n");
    printf ("  --file-alignment <size>            Set file alignment\n");
    printf ("  --image-base <address>             Set base address of the executable\n");
    printf ("  --section-alignment <size>         Set section alignment\n");
    printf ("  --subsystem <name>[:<version>]     Set required OS subsystem [& version]\n");
    printf ("  --[no-]insert-timestamp            Use a real timestamp (default) rather than zero.\n");
    printf ("                                     This makes binaries non-deterministic\n");
    printf ("  --kill-at                          Remove @nn from exported symbols\n");
    printf ("  --enable-reloc-section             Create the base relocation table\n");
    printf ("  --disable-reloc-section            Do not create the base relocation table\n");
    printf ("  --[disable-]nxcompat               Image is compatible with data execution\n");
    printf ("                                       prevention\n");
    printf ("  --convert-to-flat                  (experimental) Convert to flat file\n");
}

static void use_option (enum option_index option_index, char *arg)
{
    switch (option_index) {

        case COFF_OPTION_IGNORED:
            break;

        case COFF_OPTION_FILE_ALIGNMENT:
            {
                char *p;
                
                FileAlignment = strtoul (arg, &p, 0);
                if (FileAlignment == 0) FileAlignment = 1;
                if (*p != '\0') {
                    ld_error ("invalid file alignment number '%s'", arg);
                    break;
                }

                if (FileAlignment < 512 || FileAlignment > 0x10000 || (FileAlignment & (FileAlignment - 1))) {
                    ld_warn ("file alignment should be a power of two between 512 and 64 KiB (0x10000) inclusive according to the specification");
                }
            }
            break;

        case COFF_OPTION_IMAGE_BASE:
            {
                char *p;
                
                user_specified_base_address = strtoul (arg, &p, 0);
                if (*p != '\0') {
                    ld_error ("invalid start address number '%s'", arg);
                    break;
                }

                if (user_specified_base_address % 0x10000) {
                    ld_warn ("base address must be a multiple of 64 KiB (0x10000) according to the specification");
                }
            }
            break;

        case COFF_OPTION_SECTION_ALIGNMENT:
            {
                char *p;
                
                SectionAlignment = strtoul (arg, &p, 0);
                if (SectionAlignment == 0) SectionAlignment = 1;
                if (*p != '\0') {
                    ld_error ("invalid section alignment number '%s'", arg);
                    break;
                }

                if (SectionAlignment < FileAlignment) {
                    ld_warn ("section alignment must be greater than or equal to file alignment according to the specification");
                }
            }
            break;

        case COFF_OPTION_SUBSYSTEM:
            {
                char *p;

                Subsystem = strtoul (arg, &p, 0);
                if (*p == '\0') break;
                if (*p != ':') goto bad_subsystem;

                p++;
                MajorSubsystemVersion = strtoul (p, &p, 0);
                if (*p == '\0') break;
                if (*p != '.') goto bad_subsystem;

                p++;
                MinorSubsystemVersion = strtoul (p, &p, 0);
                if (*p == '\0') break;

            bad_subsystem:
                ld_error ("invalid subsystem type '%s'", arg);
            }
            break;

        case COFF_OPTION_INSERT_TIMESTAMP:
            insert_timestamp = 1;
            break;

        case COFF_OPTION_NO_INSERT_TIMESTAMP:
            insert_timestamp = 0;
            break;

        case COFF_OPTION_KILL_AT:
            kill_at = 1;
            break;

        case COFF_OPTION_ENABLE_RELOC_SECTION:
            generate_reloc_section = 1;
            break;

        case COFF_OPTION_DISABLE_RELOC_SECTION:
            generate_reloc_section = 0;
            break;

        case COFF_OPTION_NX_COMPAT:
            nx_compat = 1;
            break;

        case COFF_OPTION_DISABLE_NX_COMPAT:
            nx_compat = 0;
            break;

        case COFF_OPTION_CONVERT_TO_FLAT:
            convert_to_flat = 1;
            break;

    }
            
}

void coff_use_option (int option_index, char *arg)
{
    use_option (option_index, arg);
}

const struct long_option *coff_get_long_options (void)
{
    return long_options;
}
