# Released to the public domain.
#
# Anyone and anything may copy, edit, publish,
# use, compile, sell and distribute this work
# and all its parts in any form for any purpose,
# commercial and non-commercial, without any restrictions,
# without complying with any conditions
# and by any means.

# Produces Windows executable.
# Links with PDPCLIB created by makefile.msv.

CC=gccwin
CFLAGS=-O2
LD=pdld
LDFLAGS=
AS=aswin
AR=arwin
COPTS=-S $(CFLAGS) -fno-common -ansi -I../pdpclib -I../src -I../../pdcrc -D__WIN32__ -D__NOBIVA__ -D__32BIT__ -D__STATIC__

all: clean mfterm.exe

mfterm.exe: mfterm.obj
    $(LD) $(LDFLAGS) -s -o mfterm.exe ../pdpclib/p32start.obj mfterm.obj ../../pdos/pdpclib/pdpwin32.lib ../../pdos/src/kernel32.lib

.c.obj:
    $(CC) $(COPTS) -o $*.s $<
    $(AS) -o $@ $*.s
    rm -f $*.s

clean:
    rm -f *.o
    rm -f mfterm.s
