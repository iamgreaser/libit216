this is a port of this thing to C: http://bitbucket.org/jthlim/impulsetracker

licence is pending as per the source repo, expected to be somewhere between BSD and GPL

to build:

    cc -o it216 src/*.c -I src/*.h -lm

to play delicious music:

    ./it216 ~/path/to/music.it

you will need OSS or some OSS-emulation layer for sound

only has the IT loading routines ported and some cases are incomplete

currently broken in many ways though sample-mode stuff might work

have fun

