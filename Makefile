# I personally don't care if you steal this makefile. --GM

CFLAGS = -g -O2 -Isrc/include/ `sdl2-config --cflags`
LDFLAGS = -g
LIBS = -lm `sdl2-config --libs`
BINNAME_PLAYER = itplay
BINNAME_EDITOR = itedit
OBJDIR = build

include Makefile.common

