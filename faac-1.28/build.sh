#!/bin/sh
./configure --host=mipsel-linux \
CC=mipsel-linux-gcc \
LDFLAGS=-lm \
--prefix=/home/sanchrist/opt/ffmpeg/faac-1.28/build
