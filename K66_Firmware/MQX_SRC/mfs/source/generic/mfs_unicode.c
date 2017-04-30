/*HEADER**********************************************************************
*
* Copyright 2015 Freescale Semiconductor, Inc.
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale MQX RTOS License
* distributed with this Material.
* See the MQX_RTOS_LICENSE file distributed for more details.
*
* Brief License Summary:
* This software is provided in source form for you to use free of charge,
* but it is not open source software. You are allowed to use this software
* but you cannot redistribute it or derivative works of it in source form.
* The software may be used only in connection with a product containing
* a Freescale microprocessor, microcontroller, or digital signal processor.
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains Unicode support functions
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"


/*!
 * \brief Decodes single unicode character from an UTF-8 string.
 *
 * The position pointer is moved to point to the next data to be decoded.
 *
 * \param decode_pos
 * \param decode_boundary
 *
 * \return int32_t
 */
int32_t utf8_decode(char **decode_pos, char *decode_boundary)
{
    int c;
    int cont_bytes;
    uint32_t codepoint;
    unsigned char **utf8_pos = (unsigned char **)decode_pos;
    unsigned char *utf8_boundary = (unsigned char *)decode_boundary;

    if (utf8_boundary && (*utf8_pos >= utf8_boundary))
    {
        return 0; /* end of decoding */
    }

    c = **utf8_pos;
    if (c == '\0')
    {
        return 0;
    }
    (*utf8_pos)++;

    if (c < 0x80)
    {
        /* single byte code */
        return c;
    }

    if ((c & 0xE0) == 0xC0)
    {
        /* 2-byte code */
        codepoint = (c & 0x1F);
        cont_bytes = 1;
    }
    else if ((c & 0xF0) == 0xE0)
    {
        /* 3-byte code*/
        codepoint = (c & 0x0F);
        cont_bytes = 2;
    }
    else if ((c & 0xF8) == 0xF0)
    {
        /* 4-byte code*/
        codepoint = (c & 0x07);
        cont_bytes = 3;
    }
    else
    {
        /* the byte does not match encoding scheme */
        return -1;
    }

    while (cont_bytes)
    {
        if (utf8_boundary && (*utf8_pos >= utf8_boundary))
        {
            return -1; /* buffer overrun */
        }
        c = **utf8_pos;
        if ((c & 0xC0) != 0x80)
        {
            /* not a continuation byte */
            return -1;
        }
        (*utf8_pos)++;
        codepoint <<= 6;
        codepoint |= (c & 0x3F);
        cont_bytes--;
    }

    if (codepoint < 0x80 || codepoint > 0x10FFFF)
    {
        /* forbidden overlong encoding of a 7-bit character or an invalid codepoint */
        return -1;
    }

    return codepoint;
}


/*!
 * \brief Decodes UTF-8 string in reverse order.
 *
 * The position pointer is moved to next data to be decoded.
 *
 * \param decode_str
 * \param decode_pos
 *
 * \return int32_t
 */
int32_t utf8_decode_r(char *decode_str, char **decode_pos)
{
    uint32_t codepoint;
    unsigned char *utf8_str = (unsigned char *)decode_str;
    unsigned char **utf8_pos = (unsigned char **)decode_pos;

    if (*utf8_pos < utf8_str)
    {
        return 0; /* end of decoding */
    }

    /* check for single byte code */
    if (**utf8_pos < 0x80)
    {
        codepoint = **utf8_pos;
        (*utf8_pos)--;
        return codepoint;
    }

    codepoint = 0;

    /* expecting continuation byte */
    if ((**utf8_pos & 0xC0) != 0x80)
    {
        return -1;
    }

    codepoint = **utf8_pos & 0x3F;
    (*utf8_pos)--;

    if (*utf8_pos <= utf8_str)
    {
        return -1;
    }

    /* check for 2-byte code */
    if ((**utf8_pos & 0xE0) == 0xC0)
    {
        codepoint |= (**utf8_pos & 0x1F) << 6;
        (*utf8_pos)--;
        if (codepoint < 128)
        {
            /* forbidden overlong encoding of a 7-bit character */
            return -1;
        }
        return codepoint;
    }

    /* expecting continuation byte */
    if ((**utf8_pos & 0xC0) != 0x80)
    {
        return -1;
    }

    codepoint |= (**utf8_pos & 0x3F) << 6;
    (*utf8_pos)--;

    if (*utf8_pos <= utf8_str)
    {
        return -1;
    }

    /* check for 3-byte code */
    if ((**utf8_pos & 0xF0) == 0xE0)
    {
        codepoint |= (**utf8_pos & 0x0F) << 12;
        (*utf8_pos)--;
        return codepoint;
    }

    /* expecting continuation byte */
    if ((**utf8_pos & 0xC0) != 0x80)
    {
        return -1;
    }

    codepoint |= (**utf8_pos & 0x3F) << 12;
    (*utf8_pos)--;

    if (*utf8_pos <= utf8_str)
    {
        return -1;
    }

    /* check for 4-byte code */
    if ((**utf8_pos & 0xF8) == 0xF0)
    {
        codepoint |= (**utf8_pos & 0x07) << 18;
        (*utf8_pos)--;
    }
    else
    {
        return -1;
    }

    return codepoint;
}


/*!
 * \brief Encodes UTF-8 and stores it the buffer.
 *
 * The position pointer is moved behind encoded data.
 *
 * \param[in] codepoint
 * \param encode_pos
 * \param encode_boundary
 *
 * \return int
 */
int utf8_encode(uint32_t codepoint, char **encode_pos, char *encode_boundary)
{
    int i;
    int enc_len;
    uint8_t msb_mask;
    unsigned char *utf8_pos;

    if (codepoint > 0x10FFFF)
    {
        return 0;
    }
    else if (codepoint >= 0x10000)
    {
        enc_len = 4;
        msb_mask = 0xF0;
    }
    else if (codepoint >= 0x800)
    {
        enc_len = 3;
        msb_mask = 0xE0;
    }
    else if (codepoint >= 0x80)
    {
        enc_len = 2;
        msb_mask = 0xC0;
    }
    else
    {
        enc_len = 1;
        msb_mask = 0x00;
    }

    if (encode_boundary && (*encode_pos + enc_len > encode_boundary))
    {
        /* buffer overrun prevention */
        return 0;
    }

    utf8_pos = (unsigned char *)(*encode_pos) + enc_len - 1;
    for (i = 1; i < enc_len; i++)
    {
        *utf8_pos = (codepoint & 0x3F) | 0x80;
        codepoint >>= 6;
        utf8_pos--;
    }
    *utf8_pos = codepoint | msb_mask;

    *encode_pos += enc_len;
    return enc_len;
}


/*!
 * \brief Encodes UTF-8 and stores it the buffer in reverse order.
 *
 * The position pointer is moved behind encoded data.
 *
 * \param[in] codepoint
 * \param encode_pos
 * \param encode_boundary
 *
 * \return int
 */
int utf8_encode_r(uint32_t codepoint, char **encode_pos, char *encode_boundary)
{
    int i;
    int enc_len;
    uint8_t msb_mask;
    unsigned char *utf8_pos;

    if (codepoint > 0x10FFFF)
    {
        return 0;
    }
    else if (codepoint >= 0x10000)
    {
        enc_len = 4;
        msb_mask = 0xF0;
    }
    else if (codepoint >= 0x800)
    {
        enc_len = 3;
        msb_mask = 0xE0;
    }
    else if (codepoint >= 0x80)
    {
        enc_len = 2;
        msb_mask = 0xC0;
    }
    else
    {
        enc_len = 1;
        msb_mask = 0x00;
    }

    if (encode_boundary && (*encode_pos + enc_len > encode_boundary))
    {
        /* buffer overrun prevention */
        return 0;
    }

    utf8_pos = (unsigned char *)(*encode_pos);
    for (i = 1; i < enc_len; i++)
    {
        *utf8_pos = (codepoint & 0x3F) | 0x80;
        codepoint >>= 6;
        utf8_pos++;
    }
    *utf8_pos = codepoint | msb_mask;

    *encode_pos += enc_len;
    return enc_len;
}


/*!
 * \brief Bytewise reverses given memory area.
 *
 * \param first
 * \param last
 *
 * \return void
 */
void mem_reverse(char *first, char *last)
{
    char a;
    while (first < last)
    {
        a = *first;
        *first = *last;
        *last = a;
        first++;
        last--;
    }
}
