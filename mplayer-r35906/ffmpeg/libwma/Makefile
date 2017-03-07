include ../config.mak

NAME = wma
OBJS = wma_bitstream.o wma_fft_fix.o wma_mdct_fix.o wma_wmadecfix.o wma_wmafix.o 

libwma.a:$(OBJS)
	mipsel-linux-gcc -c $(OBJS) 

#include ../ffmpeg/common.mak
#include ../common.mak
