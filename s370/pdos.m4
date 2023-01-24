//PDPMVS   JOB CLASS=C,REGION=0K
//*
//PDPASM   PROC MEMBER=''
//ASM      EXEC PGM=ASMA90,
//   PARM='DECK,LIST'
//SYSLIB   DD DSN=SYS1.MACLIB,DISP=SHR
//         DD DSN=&&MACLIB,DISP=(OLD,PASS)
//         DD DSN=SYS1.MODGEN,DISP=SHR
//         DD DSN=SYS1.APVTMACS,DISP=SHR
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(20,10))
//SYSUT2   DD UNIT=SYSALLDA,SPACE=(CYL,(20,10))
//SYSUT3   DD UNIT=SYSALLDA,SPACE=(CYL,(20,10))
//SYSPRINT DD SYSOUT=*
//SYSLIN   DD DUMMY
//SYSGO    DD DUMMY
//SYSPUNCH DD DSN=&&OBJSET,UNIT=SYSALLDA,SPACE=(80,(2000,2000)),
//            DISP=(,PASS)
//*
//LKED     EXEC PGM=IEWL,PARM='NCAL',
//            COND=(4,LT,ASM)
//SYSLIN   DD DSN=&&OBJSET,DISP=(OLD,DELETE)
//SYSLMOD  DD DSN=&&NCALIB(&MEMBER),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//         PEND
//CREATE   EXEC PGM=IEFBR14
//DD0      DD DSN=&&HEX,DISP=(,PASS),
// DCB=(RECFM=U,LRECL=0,BLKSIZE=3200),
// SPACE=(CYL,(10,10,20)),UNIT=SYSALLDA
//DD12     DD DSN=&&NCALIB,DISP=(,PASS),
// DCB=(RECFM=U,LRECL=0,BLKSIZE=3200),
// SPACE=(CYL,(10,10,20)),UNIT=SYSALLDA
//DD13     DD DSN=&&LOADLIB,DISP=(,PASS),
// DCB=(RECFM=U,LRECL=0,BLKSIZE=18432),
// SPACE=(CYL,(10,10,20)),UNIT=SYSALLDA
//DD14     DD DSN=&&MACLIB,DISP=(,PASS),
// DCB=(RECFM=FB,LRECL=80,BLKSIZE=3120),
// SPACE=(CYL,(10,10,20)),UNIT=SYSALLDA
//*
//PDPTOP   EXEC PGM=IEBGENER
//SYSUT2   DD  DSN=&&MACLIB(PDPTOP),DISP=(OLD,PASS)
//SYSUT1   DD  *
undivert(pdptop.mac)dnl
/*
//SYSPRINT DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//PDPMAIN  EXEC PGM=IEBGENER
//SYSUT2   DD  DSN=&&MACLIB(PDPMAIN),DISP=(OLD,PASS)
//SYSUT1   DD  *
undivert(pdpmain.mac)dnl
/*
//SYSPRINT DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//PDPPRLG  EXEC PGM=IEBGENER
//SYSUT2   DD  DSN=&&MACLIB(PDPPRLG),DISP=(OLD,PASS)
//SYSUT1   DD  *
undivert(pdpprlg.mac)dnl
/*
//SYSPRINT DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//PDPEPIL  EXEC PGM=IEBGENER
//SYSUT2   DD  DSN=&&MACLIB(PDPEPIL),DISP=(OLD,PASS)
//SYSUT1   DD  *
undivert(pdpepil.mac)dnl
/*
//SYSPRINT DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//SAPSTART EXEC PDPASM,MEMBER=SAPSTART
//SYSIN  DD  *
undivert(sapstart.asm)dnl
/*
//SAPSUPA  EXEC PDPASM,MEMBER=SAPSUPA
//SYSIN  DD  *
undivert(sapsupa.asm)dnl
/*
//MVSSTART EXEC PDPASM,MEMBER=MVSSTART
//SYSIN  DD  *
undivert(mvsstart.asm)dnl
/*
//MVSSUPA  EXEC PDPASM,MEMBER=MVSSUPA
//SYSIN  DD  *
undivert(mvssupa.asm)dnl
/*
//START    EXEC PDPASM,MEMBER=START
//SYSIN  DD *
undivert(start.s)dnl
/*
//STDIO    EXEC PDPASM,MEMBER=STDIO
//SYSIN  DD *
undivert(stdio.s)dnl
/*
//STDLIB   EXEC PDPASM,MEMBER=STDLIB
//SYSIN  DD  *
undivert(stdlib.s)dnl
/*
//CTYPE    EXEC PDPASM,MEMBER=CTYPE
//SYSIN  DD  *
undivert(ctype.s)dnl
/*
//STRING   EXEC PDPASM,MEMBER=STRING
//SYSIN  DD  *
undivert(string.s)dnl
/*
//TIME     EXEC PDPASM,MEMBER=TIME
//SYSIN  DD  *
undivert(time.s)dnl
/*
//ERRNO    EXEC PDPASM,MEMBER=ERRNO
//SYSIN  DD  *
undivert(errno.s)dnl
/*
//ASSERT   EXEC PDPASM,MEMBER=ASSERT
//SYSIN  DD  *
undivert(assert.s)dnl
/*
//LOCALE   EXEC PDPASM,MEMBER=LOCALE
//SYSIN  DD  *
undivert(locale.s)dnl
/*
//MATH     EXEC PDPASM,MEMBER=MATH
//SYSIN  DD  *
undivert(math.s)dnl
/*
//SETJMP   EXEC PDPASM,MEMBER=SETJMP
//SYSIN  DD  *
undivert(setjmp.s)dnl
/*
//SIGNAL   EXEC PDPASM,MEMBER=SIGNAL
//SYSIN  DD  *
undivert(signal.s)dnl
/*
//@@MEMMGR EXEC PDPASM,MEMBER=@@MEMMGR
//SYSIN  DD  *
undivert(__memmgr.s)dnl
/*
//PLOAD    EXEC PDPASM,MEMBER=PLOAD
//SYSIN  DD  *
undivert(pload.s)dnl
/*
//PLOADSUP EXEC PDPASM,MEMBER=PLOADSUP
//SYSIN  DD  *
undivert(ploadsup.asm)dnl
/*
//PDOS     EXEC PDPASM,MEMBER=PDOS
//SYSIN  DD  *
undivert(pdos.s)dnl
/*
//PDOSSUP  EXEC PDPASM,MEMBER=PDOSSUP
//SYSIN  DD  *
undivert(pdossup.asm)dnl
/*
//PDOSUTIL EXEC PDPASM,MEMBER=PDOSUTIL
//SYSIN  DD  *
undivert(pdosutil.s)dnl
/*
//PCOMM    EXEC PDPASM,MEMBER=PCOMM
//SYSIN  DD  *
undivert(pcomm.s)dnl
/*
//WORLD    EXEC PDPASM,MEMBER=WORLD
//SYSIN  DD  *
undivert(world.s)dnl
/*
//MKIPLTAP EXEC PDPASM,MEMBER=MKIPLTAP
//SYSIN  DD  *
undivert(mkipltap.s)dnl
/*
//MKIPLCRD EXEC PDPASM,MEMBER=MKIPLCRD
//SYSIN  DD  *
undivert(mkiplcrd.s)dnl
/*
//BBS      EXEC PDPASM,MEMBER=BBS
//SYSIN  DD  *
undivert(bbs.s)dnl
/*
//PDPNNTP  EXEC PDPASM,MEMBER=PDPNNTP
//SYSIN  DD  *
undivert(pdpnntp.s)dnl
/*
//WTOWORLD EXEC PDPASM,MEMBER=WTOWORLD
//SYSIN  DD  *
         CSECT
         USING *,15
         WTO   'HELLO from WTO'
         LA    15,0
         BR    14
         END
/*
//*
//*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(WTOWORLD),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN    DD *
 INCLUDE SYSLIB(WTOWORLD)
/*
//*
//*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(PLOAD),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(SAPSTART)
 INCLUDE SYSLIB(START)
 INCLUDE SYSLIB(SAPSUPA)
 INCLUDE SYSLIB(STDIO)
 INCLUDE SYSLIB(STDLIB)
 INCLUDE SYSLIB(CTYPE)
 INCLUDE SYSLIB(STRING)
 INCLUDE SYSLIB(TIME)
 INCLUDE SYSLIB(ERRNO)
 INCLUDE SYSLIB(ASSERT)
 INCLUDE SYSLIB(LOCALE)
 INCLUDE SYSLIB(MATH)
 INCLUDE SYSLIB(SETJMP)
 INCLUDE SYSLIB(SIGNAL)
 INCLUDE SYSLIB(@@MEMMGR)
 INCLUDE SYSLIB(PLOAD)
 INCLUDE SYSLIB(PLOADSUP)
 INCLUDE SYSLIB(PDOSUTIL)
 ENTRY @@MAIN
/*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(PDOS),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(SAPSTART)
 INCLUDE SYSLIB(START)
 INCLUDE SYSLIB(SAPSUPA)
 INCLUDE SYSLIB(STDIO)
 INCLUDE SYSLIB(STDLIB)
 INCLUDE SYSLIB(CTYPE)
 INCLUDE SYSLIB(STRING)
 INCLUDE SYSLIB(TIME)
 INCLUDE SYSLIB(ERRNO)
 INCLUDE SYSLIB(ASSERT)
 INCLUDE SYSLIB(LOCALE)
 INCLUDE SYSLIB(MATH)
 INCLUDE SYSLIB(SETJMP)
 INCLUDE SYSLIB(SIGNAL)
 INCLUDE SYSLIB(@@MEMMGR)
 INCLUDE SYSLIB(PDOS)
 INCLUDE SYSLIB(PDOSSUP)
 INCLUDE SYSLIB(PDOSUTIL)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(PCOMM),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(MVSSTART)
 INCLUDE SYSLIB(START)
 INCLUDE SYSLIB(MVSSUPA)
 INCLUDE SYSLIB(STDIO)
 INCLUDE SYSLIB(STDLIB)
 INCLUDE SYSLIB(CTYPE)
 INCLUDE SYSLIB(STRING)
 INCLUDE SYSLIB(TIME)
 INCLUDE SYSLIB(ERRNO)
 INCLUDE SYSLIB(ASSERT)
 INCLUDE SYSLIB(LOCALE)
 INCLUDE SYSLIB(MATH)
 INCLUDE SYSLIB(SETJMP)
 INCLUDE SYSLIB(SIGNAL)
 INCLUDE SYSLIB(@@MEMMGR)
 INCLUDE SYSLIB(PCOMM)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(WORLD),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(MVSSTART)
 INCLUDE SYSLIB(START)
 INCLUDE SYSLIB(MVSSUPA)
 INCLUDE SYSLIB(STDIO)
 INCLUDE SYSLIB(STDLIB)
 INCLUDE SYSLIB(CTYPE)
 INCLUDE SYSLIB(STRING)
 INCLUDE SYSLIB(TIME)
 INCLUDE SYSLIB(ERRNO)
 INCLUDE SYSLIB(ASSERT)
 INCLUDE SYSLIB(LOCALE)
 INCLUDE SYSLIB(MATH)
 INCLUDE SYSLIB(SETJMP)
 INCLUDE SYSLIB(SIGNAL)
 INCLUDE SYSLIB(@@MEMMGR)
 INCLUDE SYSLIB(WORLD)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(MKIPLTAP),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(MVSSTART)
 INCLUDE SYSLIB(START)
 INCLUDE SYSLIB(MVSSUPA)
 INCLUDE SYSLIB(STDIO)
 INCLUDE SYSLIB(STDLIB)
 INCLUDE SYSLIB(CTYPE)
 INCLUDE SYSLIB(STRING)
 INCLUDE SYSLIB(TIME)
 INCLUDE SYSLIB(ERRNO)
 INCLUDE SYSLIB(ASSERT)
 INCLUDE SYSLIB(LOCALE)
 INCLUDE SYSLIB(MATH)
 INCLUDE SYSLIB(SETJMP)
 INCLUDE SYSLIB(SIGNAL)
 INCLUDE SYSLIB(@@MEMMGR)
 INCLUDE SYSLIB(MKIPLTAP)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(MKIPLCRD),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(MVSSTART)
 INCLUDE SYSLIB(START)
 INCLUDE SYSLIB(MVSSUPA)
 INCLUDE SYSLIB(STDIO)
 INCLUDE SYSLIB(STDLIB)
 INCLUDE SYSLIB(CTYPE)
 INCLUDE SYSLIB(STRING)
 INCLUDE SYSLIB(TIME)
 INCLUDE SYSLIB(ERRNO)
 INCLUDE SYSLIB(ASSERT)
 INCLUDE SYSLIB(LOCALE)
 INCLUDE SYSLIB(MATH)
 INCLUDE SYSLIB(SETJMP)
 INCLUDE SYSLIB(SIGNAL)
 INCLUDE SYSLIB(@@MEMMGR)
 INCLUDE SYSLIB(MKIPLCRD)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(BBS),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(MVSSTART)
 INCLUDE SYSLIB(START)
 INCLUDE SYSLIB(MVSSUPA)
 INCLUDE SYSLIB(STDIO)
 INCLUDE SYSLIB(STDLIB)
 INCLUDE SYSLIB(CTYPE)
 INCLUDE SYSLIB(STRING)
 INCLUDE SYSLIB(TIME)
 INCLUDE SYSLIB(ERRNO)
 INCLUDE SYSLIB(ASSERT)
 INCLUDE SYSLIB(LOCALE)
 INCLUDE SYSLIB(MATH)
 INCLUDE SYSLIB(SETJMP)
 INCLUDE SYSLIB(SIGNAL)
 INCLUDE SYSLIB(@@MEMMGR)
 INCLUDE SYSLIB(BBS)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(PDPNNTP),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(MVSSTART)
 INCLUDE SYSLIB(START)
 INCLUDE SYSLIB(MVSSUPA)
 INCLUDE SYSLIB(STDIO)
 INCLUDE SYSLIB(STDLIB)
 INCLUDE SYSLIB(CTYPE)
 INCLUDE SYSLIB(STRING)
 INCLUDE SYSLIB(TIME)
 INCLUDE SYSLIB(ERRNO)
 INCLUDE SYSLIB(ASSERT)
 INCLUDE SYSLIB(LOCALE)
 INCLUDE SYSLIB(MATH)
 INCLUDE SYSLIB(SETJMP)
 INCLUDE SYSLIB(SIGNAL)
 INCLUDE SYSLIB(@@MEMMGR)
 INCLUDE SYSLIB(PDPNNTP)
 ENTRY @@MAIN
/*
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=PLOAD
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&COPY2,DISP=(NEW,PASS),SPACE=(CYL,(1,1)),
//         DCB=(RECFM=U,LRECL=0,BLKSIZE=18452),UNIT=SYSALLDA
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//PLOAD    EXEC PGM=LOADZERO,PARM='dd:in dd:out'
//STEPLIB  DD  DSN=OZPD.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY2,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(PLOAD),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=PDOS
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&COPY2,DISP=(NEW,PASS),SPACE=(CYL,(1,1)),
//         DCB=(RECFM=U,LRECL=0,BLKSIZE=18452),UNIT=SYSALLDA
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//PLOAD    EXEC PGM=LOADZERO,PARM='dd:in dd:out'
//STEPLIB  DD  DSN=OZPD.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY2,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(PDOSIMG),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=PDOS
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(PDOS),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=WTOWORLD
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(WTOWORLD),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy WORLD
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=WORLD
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(WORLD),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy MKIPLTAP
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=MKIPLTAP
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(MKIPLTAP),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy MKIPLCRD
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=MKIPLCRD
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(MKIPLCRD),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy BBS
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=BBS
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(BBS),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//*
//* Copy PDPNNTP
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=PDPNNTP
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(PDPNNTP),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy DIFF.  Note that this should really be part of
//* the SEASIK package rather than here
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=DIFFUTL.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=DIFF
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(DIFF),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy UEMACS
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=UEMACS.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=UEMACS
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(UEMACS),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy PDMAKE
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=PDMAKE.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=PDMAKE
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(PDMAKE),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy EDLIN
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=EDLIN.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=EDLIN
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(EDLIN),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy GCC.
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=GCC.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,10)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=GCC
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(GCC),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy COPYFILE.
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,10)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=COPYFILE
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(COPYFILE),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy HEXDUMP.
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,10)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=HEXDUMP
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(HEXDUMP),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy MVSENDEC.
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,10)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=MVSENDEC
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(MVSENDEC),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy MVSUNZIP.
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,10)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=MVSUNZIP
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(MVSUNZIP),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=PCOMM
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(PCOMM),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//OUT      DD  DSN=&&HEX(PCOMMIN),DISP=(OLD,PASS)
//IN       DD  *
echo off
echo welcome to autoexec.bat
echo type "help" for some example commands

echo note that in order to use GCC:
rem gcc --version

echo you need a command such as:
echo gcc -S -I . -o - sample.c
echo the package should have shipped with "mvsunzip pdpi.zip" already run

echo that's enough for now - enter further commands yourself!
echo on
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//OUT      DD  DSN=&&HEX(SAMPLE),DISP=(OLD,PASS)
//IN       DD  *
#include <stdio.h>

int main(void)
{
    printf("hello, world\n");
    printf("maximum file size is %d\n", FILENAME_MAX);
    return (0);
}
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//OUT      DD  DSN=&&HEX(PDOSIN),DISP=(OLD,PASS)
//IN       DD  *
undivert(pdos.cnf)dnl
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//OUT      DD  DSN=&&HEX(ANTITWIT),DISP=(OLD,PASS)
//IN       DD  *
undivert(tweets.txt)dnl
/*
//*
//ZIP      EXEC PGM=MINIZIP,PARM='-0 -x .txt -l -o dd:out dd:in'
//STEPLIB  DD  DSN=MINIZIP.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//OUT      DD  DSN=HERC02.ZIP,DISP=(,KEEP),UNIT=TAPE,
//         LABEL=(1,SL),VOL=SER=MFTOPC,
//         DCB=(RECFM=U,LRECL=0,BLKSIZE=8000)
//SYSTERM  DD  SYSOUT=*
//SYSUT1   DD  DSN=&&TEMP,DISP=(,DELETE),UNIT=SYSALLDA,
//         SPACE=(CYL,(10,10))
//IN       DD  DSN=&&HEX,DISP=(OLD,PASS)
//*
//
