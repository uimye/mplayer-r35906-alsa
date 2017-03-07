#!/bin/sh
./configure --host=mipsel-linux \
CC=mipsel-linux-gcc \
LDFLAGS=-lm \
--prefix=/home/sanchrist/opt/ffmpeg/faad2-2.7/build
