#
# Generated by the Apple Project Builder.
#
# NOTE: Do NOT change this file -- Project Builder maintains it.
#
# Put all of your customizations in files called Makefile.preamble
# and Makefile.postamble (both optional), and Makefile will include them.
#

NAME = diskdev_cmds

PROJECTVERSION = 2.8
PROJECT_TYPE = Aggregate

TOOLS = clri.tproj dev_mkdb.tproj dump.tproj\
        dumpfs.tproj edquota.tproj fdisk.tproj fsck.tproj\
        fsck_hfs.tproj fsck_msdos.tproj mount.tproj\
        mountd.tproj mount_cd9660.tproj mount_devfs.tproj\
        mount_fdesc.tproj mount_hfs.tproj mount_nfs.tproj\
        mount_synthfs.tproj newfs.tproj newfs_hfs.tproj\
        newfs_msdos.tproj quot.tproj quota.tproj quotacheck.tproj\
        quotaon.tproj repquota.tproj restore.tproj showmount.tproj\
        tunefs.tproj umount.tproj ufs.tproj vsdbutil.tproj\
        vndevice.tproj

LIBRARIES = disklib

OTHERSRCS = Makefile Makefile.include


MAKEFILEDIR = $(MAKEFILEPATH)/pb_makefiles
CODE_GEN_STYLE = DYNAMIC
MAKEFILE = aggregate.make
LIBS = 
DEBUG_LIBS = $(LIBS)
PROF_LIBS = $(LIBS)


NEXTSTEP_PB_CFLAGS = -no-cpp-precomp


NEXTSTEP_BUILD_OUTPUT_DIR = /tmp/BUILD

NEXTSTEP_OBJCPLUS_COMPILER = /usr/bin/cc
WINDOWS_OBJCPLUS_COMPILER = $(DEVDIR)/gcc
PDO_UNIX_OBJCPLUS_COMPILER = $(NEXTDEV_BIN)/gcc
NEXTSTEP_JAVA_COMPILER = /usr/bin/javac
WINDOWS_JAVA_COMPILER = $(JDKBINDIR)/javac.exe
PDO_UNIX_JAVA_COMPILER = $(NEXTDEV_BIN)/javac

include $(MAKEFILEDIR)/platform.make

-include Makefile.preamble

include $(MAKEFILEDIR)/$(MAKEFILE)

-include Makefile.postamble

-include Makefile.dependencies
