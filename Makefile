# I personally don't care if you steal this makefile. --GM

CFLAGS = -g -O2 -Isrc/include/
LDFLAGS = -g
LIBS = -lm
BINNAME_PLAYER = itplay
BINNAME_EDITOR = itedit
OBJDIR = build

include Makefile.common

