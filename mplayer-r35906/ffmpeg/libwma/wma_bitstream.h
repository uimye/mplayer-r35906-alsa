
#ifndef BITSTREAM_H
#define BITSTREAM_H

#define ALT_BITSTREAM_READER

/* bit input buffer, buffer_end and size_in_bits must be present and used by every reader */
typedef struct GetBitContext
{
    const uint8_t *buffer,  *buffer_end;
    int index;
//    int size_in_bits;
} GetBitContext;

#define VLC_TYPE int16_t

typedef struct VLC
{
    int bits;
    VLC_TYPE(*table)[2]; ///< code, bits
    int table_size, table_allocated;
} VLC;

static inline int unaligned32_be(const void *v)
{
    const uint8_t *p = v;
    return (((p[0] << 8) | p[1]) << 16) | (p[2] << 8) | (p[3]);
}

static inline int unaligned32_le(const void *v)
{
	const uint8_t *p=v;
	return (((p[3]<<8) | p[2])<<16) | (p[1]<<8) | (p[0]);
}

/*
#define NEG_USR32(a,s) (((uint32_t)(a))>>(32-(s)))

#ifdef LITTILE_END

//#define UPDATE_CACHE(re, s)		re_cache= unaligned32_be( ((const uint8_t *)s->buffer)+(re_index>>3) ) << (re_index&0x07);
#define UPDATE_CACHE(name, gb)		name##_cache= unaligned32_be( ((const uint8_t *)(gb)->buffer)+(name##_index>>3) ) << (name##_index&0x07);\

//#define SKIP_CACHE(re, s, n)		re_cache <<= (n);
#define SKIP_CACHE(name, gb, num)	name##_cache <<= (num);

//#define SHOW_UBITS(re, s, num)	NEG_USR32(re_cache, num)  (((uint32_t)(re_cache))>>(32-(num)))
#define SHOW_UBITS(name, gb, num)	NEG_USR32(name##_cache, num)

#else

//#define UPDATE_CACHE(re, s)		re_cache= unaligned32_le( ((const uint8_t *)s->buffer)+(re_index>>3) ) >> (re_index&0x07);
#define UPDATE_CACHE(name, gb)		name##_cache= unaligned32_le( ((const uint8_t *)(gb)->buffer)+(name##_index>>3) ) >> (name##_index&0x07);\

//#define SKIP_CACHE(re, s, n)		re_cache >>= (n);
#define SKIP_CACHE(name, gb, num)	name##_cache >>= (num);

//#define SHOW_UBITS(re, gb, num)	((re_cache) & (NEG_USR32(0xffffffff,num)))     ((re_cache) & (((uint32_t)(0xffffffff))>>(32-(num))))
#define SHOW_UBITS(name, gb, num)	((name##_cache) & (NEG_USR32(0xffffffff,num)))

#endif
// */

static inline int get_bits_count(GetBitContext *s)
{
    return s->index;
}

static inline unsigned int get_bits(GetBitContext *s, int n)
{
    register int tmp;

	int re_index= (s)->index;
	int re_cache= 0;

//	UPDATE_CACHE(re, s)
//	tmp = SHOW_UBITS(re, s, n);
#ifdef LITTILE_END
	re_cache= unaligned32_be( ((const uint8_t *)s->buffer)+(re_index>>3) ) << (re_index&0x07);
	tmp=(((uint32_t)(re_cache))>>(32-n));
#else
	re_cache= unaligned32_le( ((const uint8_t *)s->buffer)+(re_index>>3) ) >> (re_index&0x07);
	tmp=((re_cache) & (((uint32_t)(0xffffffff))>>(32-(n))));
#endif

	re_index += n;

	s->index= re_index;

	return tmp;
}

static inline unsigned int get_bits1(GetBitContext *s)
{
    int index= s->index;
    uint8_t result= s->buffer[ index>>3 ];

    result<<= (index&0x07);
    result>>= 8 - 1;

    s->index ++;
	
    return result;
}

unsigned int get_bits_long(GetBitContext *s, int n);

/**
 * shows 0-17 bits.
 * Note, the alt bitstream reader can read up to 25 bits, but the libmpeg2 reader can't
 */
static inline unsigned int show_bits(GetBitContext *s, int n)
{
    register int tmp;

	int re_index= (s)->index;
	int re_cache= 0;

//	UPDATE_CACHE(re, s)
//	tmp = SHOW_UBITS(re, s, n);
#ifdef LITTILE_END
	re_cache = unaligned32_be( ((const uint8_t *)s->buffer)+(re_index>>3) );
	re_cache <<= (re_index&0x07);

	re_cache= unaligned32_be( ((const uint8_t *)s->buffer)+(re_index>>3) ) << (re_index&0x07);
	tmp=(((uint32_t)(re_cache))>>(32-n));
#else
	re_cache= unaligned32_le( ((const uint8_t *)s->buffer)+(re_index>>3) ) >> (re_index&0x07);
	tmp=((re_cache) & (((uint32_t)(0xffffffff))>>(32-(n))));
#endif

    return tmp;
}

unsigned int show_bits_long(GetBitContext *s, int n);

static inline void skip_bits(GetBitContext *s, int n)
{
    //Note gcc seems to optimize this to s->index+=n for the ALT_READER :))

	int re_index= (s)->index;
	int re_cache= 0;

//	UPDATE_CACHE(re, s)
#ifdef LITTILE_END
	re_cache= unaligned32_be( ((const uint8_t *)s->buffer)+(re_index>>3) ) << (re_index&0x07);
#else
	re_cache= unaligned32_le( ((const uint8_t *)s->buffer)+(re_index>>3) ) >> (re_index&0x07);
#endif

	re_index += n;

	s->index= re_index;
}

/**
 * init GetBitContext.
 * @param buffer bitstream buffer, must be FF_INPUT_BUFFER_PADDING_SIZE bytes larger then the actual read bits
 * because some optimized bitstream readers read 32 or 64 bit at once and could read over the end
 * @param bit_size the size of the buffer in bits
 */
static void init_get_bits(GetBitContext *s, const uint8_t *buffer, int bit_size)
{
    int buffer_size = (bit_size + 7) >> 3;
    if (buffer_size < 0 || bit_size < 0)
    {
        buffer_size = bit_size = 0;
        buffer = NULL;
    }

    s->buffer = buffer;
//    s->size_in_bits = bit_size;
    s->buffer_end = buffer + buffer_size;
    s->index = 0;

    {
		int re_index= (s)->index;
		int re_cache= 0;

//		UPDATE_CACHE(re, s)
#ifdef LITTILE_END
		re_cache= unaligned32_be( ((const uint8_t *)s->buffer)+(re_index>>3) ) << (re_index&0x07);
#else
		re_cache= unaligned32_le( ((const uint8_t *)s->buffer)+(re_index>>3) ) >> (re_index&0x07);
#endif
		
		s->index= re_index;
    }
}

#define INIT_VLC_USE_STATIC 1
#define INIT_VLC_LE         2

void align_get_bits(GetBitContext *s);

int init_vlc(VLC *vlc, int nb_bits, int nb_codes, const void *bits, int bits_wrap, int bits_size, const void *codes, int codes_wrap, int codes_size, int flags);

//void free_vlc(VLC *vlc);//Leo

/**
*
* if the vlc code is invalid and max_depth=1 than no bits will be removed
* if the vlc code is invalid and max_depth>1 than the number of bits removed
* is undefined
*/
static int get_vlc2(GetBitContext *s, VLC_TYPE(*table)[2], int bits, int max_depth)
{
    int code;
	int n, index, nb_bits;

	int re_index= (s)->index;
	int re_cache= 0;

//	UPDATE_CACHE(re, s)	
//	index= SHOW_UBITS(re, s, bits);
#ifdef LITTILE_END
	re_cache= unaligned32_be( ((const uint8_t *)s->buffer)+(re_index>>3) ) << (re_index&0x07);
	index=(((uint32_t)(re_cache))>>(32-bits));
#else
	re_cache= unaligned32_le( ((const uint8_t *)s->buffer)+(re_index>>3) ) >> (re_index&0x07);
	index= ((re_cache) & (((uint32_t)(0xffffffff))>>(32-bits)));
#endif

	code = table[index][0];
	n    = table[index][1];
	
	if(max_depth > 1 && n < 0)
	{
		re_index += bits;
		nb_bits = -n;

//		UPDATE_CACHE(re, s)
//		index= SHOW_UBITS(re, s, nb_bits) + code;
#ifdef LITTILE_END
		re_cache= unaligned32_be( ((const uint8_t *)s->buffer)+(re_index>>3) ) << (re_index&0x07);
		index=(((uint32_t)(re_cache))>>(32-nb_bits)) + code;
#else
		re_cache= unaligned32_le( ((const uint8_t *)s->buffer)+(re_index>>3) ) >> (re_index&0x07);
		index= ((re_cache) & (((uint32_t)(0xffffffff))>>(32-nb_bits))) + code;
#endif
		code = table[index][0];
		n    = table[index][1];
		if(max_depth > 2 && n < 0)
		{
			re_index += nb_bits;
	
			nb_bits = -n;

//			UPDATE_CACHE(re, s)
//			index= SHOW_UBITS(re, s, nb_bits) + code;
#ifdef LITTILE_END
			re_cache= unaligned32_be( ((const uint8_t *)s->buffer)+(re_index>>3) ) << (re_index&0x07);
			index=(((uint32_t)(re_cache))>>(32-nb_bits)) + code;
#else
			re_cache= unaligned32_le( ((const uint8_t *)s->buffer)+(re_index>>3) ) >> (re_index&0x07);
			index= ((re_cache) & (((uint32_t)(0xffffffff))>>(32-nb_bits))) + code;
#endif
			code = table[index][0];
			n    = table[index][1];
		}
	}

//	SKIP_CACHE(re, s, n)
#ifdef LITTILE_END
	re_cache <<= (n);
#else
	re_cache >>= (n);
#endif

	re_index += n;

	s->index = re_index;
	
	return code;
}

#endif
