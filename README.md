this is a port of this thing to C: http://bitbucket.org/jthlim/impulsetracker

licence now exists, check LICENCE.TXT

to build all the things:

    make

to build player manually:

    cc -o itplay src/sdriver/*.c src/player/*.c src/player.c -I src/include/ -lm

to build editor manually:

    cc -o itedit -DEDITOR src/sdriver/*.c src/player/*.c src/ui/*.c -I src/include/ -lm

to play delicious music:

    ./itplay ~/path/to/music.it

you will need OSS or some OSS-emulation layer for sound

only has the IT loading routines ported and some cases are incomplete

currently broken in many ways though sample-mode stuff might work

have fun

