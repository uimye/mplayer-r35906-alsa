
#include "wma_avcodec.h"
#include "wma_bitstream.h"
#include "../libavutil/mem.h"

/**
 * reads 0-32 bits.
 */
unsigned int get_bits_long(GetBitContext *s, int n)
{
    if (n <= 17)
        return get_bits(s, n);
    else
    {
        int ret = get_bits(s, 16) << (n - 16);
        return ret | get_bits(s, n - 16);
    }
}

/**
 * shows 0-32 bits.
 */
unsigned int show_bits_long(GetBitContext *s, int n)
{
    if (n <= 17)
        return show_bits(s, n);
    else
    {
        GetBitContext gb =  *s;
        int ret = get_bits_long(s, n);
        *s = gb;
        return ret;
    }
}

void align_get_bits(GetBitContext *s)
{
    int n = ( - get_bits_count(s)) &7;
    if (n)
        skip_bits(s, n);
}

/* VLC decoding */

#define GET_DATA(v, table, i, wrap, size) \
{\
	ptr = (uint8_t *)table + i * wrap;\
	switch(size) {\
	case 1:\
		v = *(uint8_t *)ptr;\
		break;\
	case 2:\
		v = *(uint16_t *)ptr;\
		break;\
	default:\
		v = *(const uint32_t *)ptr;\
		break;\
	}\
}

static int alloc_table(VLC *vlc, int size, int use_static)
{
    int index;
    index = vlc->table_size;
    vlc->table_size += size;
    if (vlc->table_size > vlc->table_allocated)
    {
        vlc->table_allocated += (1 << vlc->bits);
        if (use_static)
//            vlc->table = av_realloc_static(vlc->table, sizeof(VLC_TYPE) *2 * vlc->table_allocated);//Leo
			printf ("+++++++++++++++-----\n");
        else
            vlc->table = av_realloc(vlc->table, sizeof(VLC_TYPE) *2 * vlc->table_allocated);
        if (!vlc->table)
            return  - 1;
    }
    return index;
}

static int build_table(VLC *vlc, int table_nb_bits, int nb_codes, const void *bits, int bits_wrap, int bits_size, const void *codes, int codes_wrap, int codes_size, uint32_t code_prefix, int n_prefix, int flags)
{
    int i, j, k, n, table_size, table_index, nb, n1, index, code_prefix2;
    uint32_t code;
    VLC_TYPE(*table)[2];

	uint8_t *ptr;

    table_size = 1 << table_nb_bits;
    table_index = alloc_table(vlc, table_size, flags &INIT_VLC_USE_STATIC);

    if (table_index < 0)
        return  - 1;
    table = &vlc->table[table_index];

    for (i = 0; i < table_size; i++)
    {
        table[i][1] = 0; //bits
        table[i][0] =  - 1; //codes
    }

    /* first pass: map codes and compute auxillary table sizes */
    for (i = 0; i < nb_codes; i++)
    {
        GET_DATA(n, bits, i, bits_wrap, bits_size);
        GET_DATA(code, codes, i, codes_wrap, codes_size);
        /* we accept tables with holes */
        if (n <= 0)
            continue;

        /* if code matches the prefix, it is in the table */
        n -= n_prefix;
        if (flags &INIT_VLC_LE)
            code_prefix2 = code &(n_prefix >= 32 ? 0xffffffff : (1 << n_prefix) - 1);
        else
            code_prefix2 = code >> n;
        if (n > 0 && code_prefix2 == code_prefix)
        {
            if (n <= table_nb_bits)
            {
                /* no need to add another table */
                j = (code << (table_nb_bits - n)) &(table_size - 1);
                nb = 1 << (table_nb_bits - n);
                for (k = 0; k < nb; k++)
                {
                    if (flags &INIT_VLC_LE)
                        j = (code >> n_prefix) + (k << n);

                    if (table[j][1] /*bits*/ != 0)
                    {
                        return  - 1;
                    }
                    table[j][1] = n; //bits
                    table[j][0] = i; //code
                    j++;
                }
            }
            else
            {
                n -= table_nb_bits;
                j = (code >> ((flags &INIT_VLC_LE) ? n_prefix : n)) &((1 << table_nb_bits) - 1);

                /* compute table size */
                n1 =  - table[j][1]; //bits
                if (n > n1)
                    n1 = n;
                table[j][1] =  - n1; //bits
            }
        }
    }

    /* second pass : fill auxillary tables recursively */
    for (i = 0; i < table_size; i++)
    {
        n = table[i][1]; //bits
        if (n < 0)
        {
            n =  - n;
            if (n > table_nb_bits)
            {
                n = table_nb_bits;
                table[i][1] =  - n; //bits
            }
            index = build_table(vlc, n, nb_codes, bits, bits_wrap, bits_size, codes, codes_wrap, codes_size, (flags &INIT_VLC_LE) ? (code_prefix | (i << n_prefix)): ((code_prefix << table_nb_bits) | i), n_prefix + table_nb_bits, flags);
            if (index < 0)
                return  - 1;
            /* note: realloc has been done, so reload tables */
            table = &vlc->table[table_index];
            table[i][0] = index; //code
        }
    }
    return table_index;
}


/* Build VLC decoding tables suitable for use with get_vlc().

'nb_bits' set thee decoding table size (2^nb_bits) entries. The
bigger it is, the faster is the decoding. But it should not be too
big to save memory and L1 cache. '9' is a good compromise.

'nb_codes' : number of vlcs codes

'bits' : table which gives the size (in bits) of each vlc code.

'codes' : table which gives the bit pattern of of each vlc code.

'xxx_wrap' : give the number of bytes between each entry of the
'bits' or 'codes' tables.

'xxx_size' : gives the number of bytes of each entry of the 'bits'
or 'codes' tables.

'wrap' and 'size' allows to use any memory configuration and types
(byte/word/long) to store the 'bits' and 'codes' tables.

'use_static' should be set to 1 for tables, which should be freed
with av_free_static(), 0 if free_vlc() will be used.
 */
int init_vlc(VLC *vlc, int nb_bits, int nb_codes, const void *bits, int bits_wrap, int bits_size, const void *codes, int codes_wrap, int codes_size, int use_static)
{
    vlc->bits = nb_bits;
    if (!use_static)
    {
        vlc->table = NULL;
        vlc->table_allocated = 0;
        vlc->table_size = 0;
    }
    else
    {
        /* Static tables are initially always NULL, return
        if vlc->table != NULL to avoid double allocation */
        if (vlc->table)
            return 0;
    }

    if (build_table(vlc, nb_bits, nb_codes, bits, bits_wrap, bits_size, codes, codes_wrap, codes_size, 0, 0, use_static) < 0)
    {
        av_free(vlc->table);
        return  - 1;
    }
    return 0;
}

static void free_vlc(VLC *vlc)
{
    av_free(vlc->table);
}
