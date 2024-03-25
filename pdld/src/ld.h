/******************************************************************************
 * @file            ld.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stddef.h>
#include <stdio.h>

#if defined (_WIN32)
# define NORETURN __declspec (noreturn)
#else
# define NORETURN
#endif

#define LD_MAJOR_VERSION 0
#define LD_MINOR_VERSION 5

#define FAKE_LD_FILENAME "autogenerated"

enum ld_oformat {
    LD_OFORMAT_COFF,
    LD_OFORMAT_ELF,
    LD_OFORMAT_LX
};

typedef unsigned long address_type;
typedef unsigned long flag_int;

extern const char *program_name;

struct ld_state {
    const char *entry_symbol_name;
    const char *output_map_filename;
    const char *output_filename;
    enum ld_oformat oformat;
    const char *output_implib_filename;
    int emit_relocs;
    int create_shared_library;
    int no_input_files_is_fine;
    
    address_type base_address;
    address_type entry_point;

    int use_custom_base_address;

    int bits;
};

extern struct ld_state *ld_state;

struct symbol {

    char *name;
    flag_int flags;
    address_type value;
    address_type size;
    struct section_part *part;
    
    long section_number; /* 1-based, 0 means undefined, negative numbers have special meaning. */

    int auxiliary; /* Such symbol should be ignored and is only a filler. */

};

#define SYMBOL_FLAG_EXCLUDE_EXPORT (1U << 0)

#define UNDEFINED_SECTION_NUMBER 0
#define ABSOLUTE_SECTION_NUMBER (-1)
#define DEBUG_SECTION_NUMBER (-2)

struct reloc_howto {
    int size;
    int pc_relative;
    int no_base;

    void *special_function; /* For special relocations. */

    const char *name;
};

enum {
    RELOC_TYPE_IGNORED,
    
    RELOC_TYPE_64,
    RELOC_TYPE_32,
    
    RELOC_TYPE_PC32,
    
    RELOC_TYPE_32_NO_BASE,
    
    RELOC_TYPE_END
};

/* link.c */
extern const struct reloc_howto reloc_howtos[RELOC_TYPE_END];

struct reloc_entry {
    struct symbol *symbol;
    address_type offset;
    address_type addend;
    const struct reloc_howto *howto;
};

struct object_file {

    struct object_file *next;

    char *filename;

    struct symbol *symbol_array;
    size_t symbol_count;

};

struct section_part {

    struct section_part *next;

    struct section *section;
    struct object_file *of;

    unsigned char *content;
    address_type content_size;
    address_type alignment;

    struct reloc_entry *relocation_array;
    size_t relocation_count;

    address_type rva;

};

struct subsection {

    struct subsection *next;

    char *name;

    struct section_part *first_part;
    struct section_part **last_part_p;

};

struct section {

    struct section *next;

    char *name;
    flag_int flags;

    struct section_part *first_part;
    struct section_part **last_part_p;

    struct subsection *all_subsections;

    address_type total_size;
    int is_bss;

    address_type rva;
    address_type section_alignment;

    int target_index;
    void *object_dependent_data;

};

#define SECTION_FLAG_ALLOC      (1U << 0)
#define SECTION_FLAG_LOAD       (1U << 1)
#define SECTION_FLAG_READONLY   (1U << 2)
#define SECTION_FLAG_CODE       (1U << 3)
#define SECTION_FLAG_DATA       (1U << 4)
#define SECTION_FLAG_NEVER_LOAD (1U << 5)
#define SECTION_FLAG_DEBUGGING  (1U << 6)
#define SECTION_FLAG_EXCLUDE    (1U << 7)
#define SECTION_FLAG_NOREAD     (1U << 8)
#define SECTION_FLAG_SHARED     (1U << 9)

extern struct section *all_sections;
extern struct object_file *all_object_files;

#define ALIGN(to_align, alignment) \
 (((to_align) / (alignment) + (((to_align) % (alignment)) ? 1 : 0)) * (alignment))

/* error.c */
NORETURN void ld_internal_error_at_source (const char *code_filename,
                                           unsigned long code_line_number,
                                           const char *fmt,
                                           ...);
NORETURN void ld_fatal_error (const char *fmt, ...);
void ld_error (const char *fmt, ...);
void ld_warn (const char *fmt, ...);
void ld_note (const char *fmt, ...);
int ld_get_error_count (void);

/* libld.c */
char **ld_parse_args (int argc, char **argv, int start_index);
int read_file_into_memory (const char *filename, unsigned char **memory_p, size_t *size_p);

/* link.c */
void link (void);

/* coff.c */
void coff_get_stub_file (unsigned char **stub_file_p, size_t *stub_size_p);
unsigned long coff_get_SizeOfStackCommit (void);

address_type coff_get_base_address (void);
address_type coff_get_first_section_rva (void);
void coff_before_link (void);
address_type coff_calculate_entry_point (void);
void coff_after_link (void);
void coff_write (const char *filename);
int coff_read (unsigned char *file, size_t file_size, const char *filename);

unsigned short coff_get_wanted_Machine (void);
void coff_archive_end (void);

void coff_print_help (void);
void coff_use_option (int option_index, char *arg);
const struct long_option *coff_get_long_options (void);

/* elf.c */
address_type elf_get_first_section_rva (void);
void elf_write (const char *filename);
int elf_read (unsigned char *file, size_t file_size, const char *filename);

/* lx.c */
void lx_import_generate_import_with_dll_name (const char *import_name,
                                              short OrdinalHint,
                                              short ImportType,
                                              short ImportNameType,
                                              const char *filename,
                                              const char *dll_name);
address_type lx_get_base_address (void);
void lx_before_link (void);
void lx_write (const char *filename);

void lx_print_help (void);
void lx_use_option (int option_index, char *arg);
const struct long_option *lx_get_long_options (void);

/* map.c */
void map_write (const char *filename);

/* read.c */
#define INPUT_FILE_NO_NEW_SYMBOLS  0
#define INPUT_FILE_FINISHED        1
#define INPUT_FILE_ERROR           2
#define INPUT_FILE_UNRECOGNIZED    3
void read_input_file (const char *filename);

/* sections.c */
struct section *section_find (const char *name);
struct section *section_find_or_make (const char *name);
void section_write (struct section *section, unsigned char *memory);
int section_count (void);

struct subsection *subsection_find (struct section *section, const char *name);
struct subsection *subsection_find_or_make (struct section *section, const char *name);

struct section_part *section_part_new (struct section *section, struct object_file *of);
void section_append_section_part (struct section *section, struct section_part *part);
void subsection_append_section_part (struct subsection *subsection, struct section_part *part);

struct object_file *object_file_make (size_t symbol_count, const char *filename);

void sections_destroy (void);
void sections_destroy_empty_before_collapse (void);

/* symbols.c */
void symbols_init (void);
void symbols_destroy (void);
struct symbol *symbol_find (const char *name);
void symbol_add_to_hashtab (struct symbol *symbol);
void symbol_remove_from_hashtab (struct symbol *symbol);
void symbol_record_external_symbol (struct symbol *symbol);
int symbol_is_undefined (const struct symbol *symbol);
address_type symbol_get_value_with_base (const struct symbol *symbol);
address_type symbol_get_value_no_base (const struct symbol *symbol);
void symbols_for_each_global (void (*symbol_callback) (struct symbol *));
