NAME = VojnaBarv
CC = gcc
CFLAGS = -O2 -std=gnu99 -Wall
SRCDIR = VojnaBarv
BUILDDIR = linux/build
BINDIR = linux/bin
TARGET = $(BINDIR)/$(NAME)

SOURCES := $(wildcard $(SRCDIR)/*.c)
# remove file with rendering code
SOURCES := $(filter-out $(SRCDIR)/render.c, $(SOURCES))
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: all clean clean_all prepare_build copy_grids

all: $(TARGET)

clean:
	rm -rf linux/build/* $(TARGET)

clean_all:
	rm -rf linux

prepare_build:
	mkdir -p linux
	mkdir -p linux/build
	mkdir -p linux/bin

copy_grids:
	cp grid_files/* linux/bin
