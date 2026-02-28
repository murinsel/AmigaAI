# AmigaAI Makefile - Cross-Compilation with m68k-amigaos-gcc
# Requires: https://github.com/bebbo/amiga-gcc
#
# Build via Docker:  ./build.sh
# Build natively:    make (if m68k-amigaos-gcc is in PATH)

PREFIX  = m68k-amigaos
CC      = $(PREFIX)-gcc
STRIP   = $(PREFIX)-strip

# CPU target: 68020+
CFLAGS  = -m68020 -O2 -Wall -noixemul -fcommon \
          -Isdk/include -Isrc
LDFLAGS = -noixemul -Lsdk/lib -Wl,--allow-multiple-definition
LIBS    = -lamisslstubs -lsocket -lm

TARGET  = AmigaAI
SRCDIR  = src
OBJDIR  = obj

SOURCES = $(SRCDIR)/main.c \
          $(SRCDIR)/http.c \
          $(SRCDIR)/claude.c \
          $(SRCDIR)/json_utils.c \
          $(SRCDIR)/cJSON.c \
          $(SRCDIR)/gui.c \
          $(SRCDIR)/arexx_port.c \
          $(SRCDIR)/config.c \
          $(SRCDIR)/memory.c

OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

.PHONY: all clean install

all: $(OBJDIR) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

# After copying to Amiga, run: AmigaAI CREATEICON
# to generate the Workbench icon.
