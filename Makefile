#	Makefile
#	Part of LisaFilesystem.
#
#	Copyright © 2026 Base Hit Ventures, LLC. All rights reserved.

.POSIX:


### Build Settings

CONFIGURATION = Release

CC = clang
RM = rm -f

OBJDIR = obj
SRCDIR = src

OPTIMIZATION_CFLAGS_Debug   = -O0
OPTIMIZATION_CFLAGS_Release = -Os
OPTIMIZATION_CFLAGS = $(OPTIMIZATION_CFLAGS_$(CONFIGURATION))

GENDEBUG_CFLAGS_Debug   = -g
GENDEBUG_CFLAGS_Release =
GENDEBUG_CFLAGS = $(GENDEBUG_CFLAGS_$(CONFIGURATION))

WARNING_CFLAGS_Debug   = -Wall -Werror
WARNING_CFLAGS_Release = -Wall
WARNING_CFLAGS = $(WARNING_CFLAGS_$(CONFIGURATION))

HEADER_SEARCH_PATHS = -I$(SRCDIR)

CFLAGS = \
	$(OPTIMIZATION_CFLAGS) \
	$(GENDEBUG_CFLAGS) \
	$(WARNING_CFLAGS) \
	$(HEADER_SEARCH_PATHS)

PREPROCESSOR_MACROS_Debug   = -DDEBUG=1
PREPROCESSOR_MACROS_Release = -DNDEBUG=1
PREPROCESSOR_MACROS = $(PREPROCESSOR_MACROS_$(CONFIGURATION))

CPPFLAGS = $(PREPROCESSOR_MACROS)


### Object Files

OBJECTS = \
	$(OBJDIR)/image_dc42.o \
	$(OBJDIR)/io_utils.o \
	$(OBJDIR)/lisafs_main.o \
	$(OBJDIR)/lisafs.o


### Build Rules

.SUFFIXES: .c .o

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<


### Primary Targets

all: $(OBJDIR) lisafs

clean:
	$(RM) lisafs
	$(RM) $(OBJECTS)

lisafs: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)


### Utility Targets

.PHONY: $(OBJDIR)
$(OBJDIR):
	@mkdir -p $(OBJDIR)


### File Dependencies

src/image_dc42.c: src/image_dc42.h \
				  src/lisafs_defines.h

src/io_utils.c: src/io_utils.h \
				src/lisafs_defines.h

src/lisafs_main.c: src/lisafs.h \
				   src/lisafs_defines.h \
				   src/image_dc42.h

src/lisafs.c: src/lisafs.h \
			  src/lisafs_defines.h \
			  src/image_dc42.h
