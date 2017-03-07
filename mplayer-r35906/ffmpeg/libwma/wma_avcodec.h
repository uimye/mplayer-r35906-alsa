
#ifndef AVCODEC_H
#define AVCODEC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wma_common.h"

//#pragma warning (disable:4018 4101)
//Leo
//#define ASF_CODEC_ID_WMAV1 0x160
//#define ASF_CODEC_ID_WMAV2 0x161

enum CodecID 
{
    CODEC_ID_NONE,
    CODEC_ID_WMAV1 = 0x160,
    CODEC_ID_WMAV2 = 0x161
};

enum CodecType
{
	CODEC_TYPE_UNKOWN,
	CODEC_TYPE_AUDIO
};

typedef struct AVCodecContext 
{
    int bit_rate;

    int flags;

    unsigned char *extradata;
    int extradata_size;

    int width, height;

    int frame_size;
    int frame_number;   ///< audio or video frame number

    int sample_rate; ///< samples per sec
    int channels;
    int bits_per_sample;
    int block_align;

    struct AVCodec *codec;

    void *priv_data;

    char codec_name[32];
//    enum CodecType codec_type; /* see CODEC_TYPE_xxx */
    enum CodecID codec_id; /* see CODEC_ID_xxx */
    unsigned int codec_tag;

    int flags2;

} AVCodecContext;

typedef struct AVCodec 
{
    const char *name;
    enum CodecType type;
    enum CodecID id;
    int priv_data_size;
    int (*init)(AVCodecContext *);
    int (*encode)(AVCodecContext *, uint8_t *buf, int buf_size, void *data);
    int (*close)(AVCodecContext *);
    int (*decode)(AVCodecContext *, void *outdata, int *outdata_size, uint8_t *buf, int buf_size);
    int capabilities;

    struct AVCodec *next;
    void (*flush)(AVCodecContext *);
//    const AVRational *supported_framerates; ///array of supported framerates, or NULL if any, array is terminated by {0,0}
//    const enum PixelFormat *pix_fmts;       ///array of supported pixel formats, or NULL if unknown, array is terminanted by -1
} AVCodec;

extern AVCodec wmav1_decoder;
extern AVCodec wmav2_decoder;

extern AVCodec *first_avcodec;

void avcodec_init(void);

void register_avcodec(AVCodec *format);

AVCodec *avcodec_find_decoder(enum CodecID id);

void avcodec_get_context_defaults(AVCodecContext *s);

AVCodecContext *avcodec_alloc_context(void);

int avcodec_open(AVCodecContext *avctx, AVCodec *codec);

int avcodec_decode_audio(AVCodecContext *avctx, int16_t *samples, int *frame_size_ptr, uint8_t *buf, int buf_size);

int avcodec_close(AVCodecContext *avctx);

void avcodec_register_all(void);

void avcodec_flush_buffers(AVCodecContext *avctx);

typedef struct AVCodecParserContext
{  
    struct AVCodecParser *parser;
} AVCodecParserContext;

typedef struct AVCodecParser 
{
    int (*split)(AVCodecContext *avctx, const uint8_t *buf, int buf_size);
} AVCodecParser;

void *av_realloc_static(void *ptr, unsigned int size);
#if 0
/* memory */
void *av_malloc(unsigned int size);
void *av_mallocz(unsigned int size);
void *av_realloc(void *ptr, unsigned int size);
void av_free(void *ptr);
void av_freep(void *ptr);
void *av_fast_realloc(void *ptr, unsigned int *size, unsigned int min_size);
/* for static data only */
/* call av_free_static to release all staticaly allocated tables */
void av_free_static(void);
void *av_mallocz_static(unsigned int size);
#endif
#ifdef __cplusplus
}
#endif

#endif
