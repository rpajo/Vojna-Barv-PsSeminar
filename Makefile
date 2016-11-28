NAME = VojnaBarv
CFLAGS = -O2 -std=c99 -Wall
SRCDIR = VojnaBarv
OBJDIR = linux/build
BINDIR = linux/bin
TARGET = $(BINDIR)/$(NAME)

SOURCES := $(wildcard $(SRCDIR)/*.c)
# remove file with rendering code
SOURCES := $(filter-out $(SRCDIR)/render.c, $(SOURCES))
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

$(TARGET): $(OBJECTS)
	gcc $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	gcc $(CFLAGS) -c -o $@ $<

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf linux/build/* linux/bin/*

init:
	mkdir linux
	mkdir linux/build
	mkdir linux/bin
