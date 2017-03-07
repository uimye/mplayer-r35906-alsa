
#ifndef COMMON_H
#define COMMON_H

#define inline __inline

#ifndef NULL
#define NULL (void*)0
#endif

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if 1
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#endif

#define LITTILE_END

#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFMAX(a,b) ((a) > (b) ? (a) : (b))

#define AV_RL16(a) (*((uint16_t*)(a)))
#define AV_RL32(a) (*((uint32_t*)(a)))

#if defined(SYMBIAN_OPENC)
#define DECLARE_ALIGNED(n,t,v)      t v __attribute__ ((aligned (n)))
#define DECLARE_ASM_CONST(n,t,v)    static const t v attribute_used __attribute__ ((aligned (n)))
#define DECLARE_ALIGNED_16(t,v)     DECLARE_ALIGNED(16,t,v)
#else 
#define DECLARE_ALIGNED_16(t,v)    t v
#endif

#if 0
static inline uint8_t av_clip_uint8(int a)
{
    if (a&(~255)) return (-a)>>31;
    else          return a;
}
static inline int16_t av_clip_int16(int a)
{
    if ((a+32768) & ~65535) return (a>>31) ^ 32767;
    else                    return a;
}
#endif

#endif
