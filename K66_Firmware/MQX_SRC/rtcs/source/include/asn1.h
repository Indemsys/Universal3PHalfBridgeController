#ifndef __asn1_h__
#define __asn1_h__
/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
* Copyright 1989-2008 ARC International
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
*   This file contains definitions for parsing the ASN.1
*   subset used by SNMP.
*
*
*END************************************************************************/


/***************************************
**
** Constants
**
*/

/*
** ASN.1 types
*/

#define ASN1_TYPE_INTEGER           0x02
#define ASN1_TYPE_OCTET             0x04
#define ASN1_TYPE_NULL              0x05
#define ASN1_TYPE_OBJECT            0x06
#define ASN1_TYPE_SEQUENCE          0x30

#define ASN1_TYPE_IpAddress         0x40
#define ASN1_TYPE_NetworkAddress    ASN1_TYPE_IpAddress
#define ASN1_TYPE_Counter           0x41
#define ASN1_TYPE_Gauge             0x42
#define ASN1_TYPE_TimeTicks         0x43
#define ASN1_TYPE_Opaque            0x44
/* CR 916 */
#define ASN1_TYPE_Counter64         0x46
/* End CR 916 */

#define ASN1_TYPE_ERR_NOOBJECT      0x80
#define ASN1_TYPE_ERR_NOINST        0x81
#define ASN1_TYPE_ERR_ENDOFMIB      0x82

#define ASN1_TYPE_PDU_GET           0xA0
#define ASN1_TYPE_PDU_GETNEXT       0xA1
#define ASN1_TYPE_PDU_RESPONSE      0xA2
#define ASN1_TYPE_PDU_SET           0xA3
#define ASN1_TYPE_PDU_TRAP          0xA4
#define ASN1_TYPE_PDU_GETBULK       0xA5
#define ASN1_TYPE_PDU_INFORM        0xA6
#define ASN1_TYPE_PDU_TRAPV2        0xA7
#define ASN1_TYPE_PDU_REPORT        0xA8


/***************************************
**
** Code macros
**
*/

#define ASN1_READ_IGNORE(p,len) \
      (p)->inbuf += len;  \
      (p)->inlen -= len

#define ASN1_READ_INT(p,len,val) \
      {                                    \
         uint32_t i = len;                  \
         if ((p)->inlen < i) return FALSE; \
         (p)->inlen -= i;                  \
         val = 0;                          \
         while (i--) {                     \
            val <<= 8;                     \
            val += *(p)->inbuf++;          \
         }                                 \
      }

#define ASN1_READ_TYPELEN(p,typecode,typelen) \
      if ((p)->inlen < 2) return FALSE;               \
      (p)->inlen -= 2;                                \
      typecode = *(p)->inbuf++;                       \
      if (*(p)->inbuf < 0x80) {                       \
         typelen = *(p)->inbuf++;                     \
      } else {                                        \
         ASN1_READ_INT(p,*(p)->inbuf++&0x7F,typelen); \
      }                                               \
      if ((p)->inlen < typelen) return FALSE

#define ASN1_READ_TYPELEN_EXPECT(p,typecode,typelen) \
      if ((p)->inlen < 2) return FALSE;                 \
      (p)->inlen -= 2;                                  \
      if (*(p)->inbuf++ != typecode) return FALSE;      \
      if (*(p)->inbuf < 0x80) {                         \
         typelen = *(p)->inbuf++;                       \
      } else {                                          \
         ASN1_READ_INT(p,(*(p)->inbuf++&0x7F),typelen); \
      }                                                 \
      if ((p)->inlen < typelen) return FALSE

#define ASN1_READZERO_IGNORE(p,len) \
      {                                    \
         uint32_t i = len;                  \
         if ((p)->inlen < i) return FALSE; \
         (p)->inlen -= i;                  \
         while (i--) {                     \
            *(p)->inbuf++ = 0;             \
         }                                 \
      }

#define ASN1_READZERO_INT(p,len,val) \
      {                                    \
         uint32_t i = len;                  \
         if ((p)->inlen < i) return FALSE; \
         (p)->inlen -= i;                  \
         val = 0;                          \
         while (i--) {                     \
            val <<= 8;                     \
            val += *(p)->inbuf;            \
            *(p)->inbuf++ = 0;             \
         }                                 \
      }

/*
** Warning:  In the BKWRITE_TYPELEN macro,
** we can have writelen == typelen
*/

#define ASN1_BKWRITE_TYPELEN(p,writelen,typecode,typelen) \
      if (typelen < 0x80) {           \
         (p)->outlen -= 2;            \
         *--(p)->outbuf = typelen;    \
         writelen += 2;               \
      } else if (typelen < 0x100) {   \
         (p)->outlen -= 3;            \
         *--(p)->outbuf = typelen;    \
         writelen += 3;               \
         *--(p)->outbuf = 0x81;       \
      } else {                        \
         (p)->outlen -= 4;            \
         (p)->outbuf -= 2;            \
         (void) mqx_htons((p)->outbuf, typelen); \
         writelen += 4;               \
         *--(p)->outbuf = 0x82;       \
      }                               \
      *--(p)->outbuf = typecode

#define ASN1_BKWRITE_TYPELEN_BYTES(p,writelen,typecode,valp,vallen) \
      (p)->outlen -= vallen;                            \
      (p)->outbuf -= vallen;                            \
      _mem_copy(valp, (p)->outbuf, vallen);             \
      ASN1_BKWRITE_TYPELEN(p,writelen,typecode,vallen); \
      writelen += vallen

#define ASN1_BKWRITE_TYPELEN_INT(p,writelen,val) \
      (p)->outlen -= 3;                  \
      *--(p)->outbuf = val;              \
      writelen += 3;                     \
      *--(p)->outbuf = 0x01;             \
      *--(p)->outbuf = ASN1_TYPE_INTEGER

/* CR 850 */
#define ASN1_BKWRITE_TYPELEN_BIGINT(p,writelen,typecode,val) \
      if (val < 0) {                      \
         uint32_t vallen = 0;              \
         for (;; val = ~(~val >> 8)) {    \
            vallen++;                     \
            *--(p)->outbuf = val & 0xFF;  \
            if (val >= -0x80) {           \
               *--(p)->outbuf = vallen;   \
               *--(p)->outbuf = typecode; \
               (p)->outlen -= vallen+2;   \
               writelen += vallen+2;      \
               break;                     \
            }                             \
         }                                \
      } else {                            \
         ASN1_BKWRITE_TYPELEN_BIGUINT(p,writelen,typecode,val); \
      }

#define ASN1_BKWRITE_TYPELEN_BIGUINT(p,writelen,typecode,val) \
      {                                   \
         uint32_t vallen = 0;              \
         for (;; val >>= 8) {             \
            vallen++;                     \
            *--(p)->outbuf = val & 0xFF;  \
            if (val < 0x80) {             \
               *--(p)->outbuf = vallen;   \
               *--(p)->outbuf = typecode; \
               (p)->outlen -= vallen+2;   \
               writelen += vallen+2;      \
               break;                     \
            }                             \
         }                                \
      }
/* End CR 850 */

#define ASN1_BKWRITE_ID(p,writelen,id) \
      {                                         \
         uint32_t i = id;                        \
         (p)->outlen--;                         \
         *--(p)->outbuf = i & 0x7F;             \
         i >>= 7;                               \
         writelen++;                            \
         while (i) {                            \
            (p)->outlen--;                      \
            *--(p)->outbuf = (i & 0x7F) | 0x80; \
            i >>= 7;                            \
            writelen++;                         \
         }                                      \
      }


/*
** These macros are the only ones useful to
** MIB writers.
*/

#define ASN1_READ_ID(p,val) \
      val = 0;                             \
      if ((p)->inlen < 1) return FALSE;    \
      (p)->inlen -= 1;                     \
      while (*(p)->inbuf & 0x80) {         \
         if ((p)->inlen < 1) return FALSE; \
         (p)->inlen -= 1;                  \
         val <<= 7;                        \
         val += *(p)->inbuf++ & 0x7F;      \
      }                                    \
      val <<= 7;                           \
      val += *(p)->inbuf++

#define ASN1_WRITE_ID(p,id) \
      if ((p)->outlen < 1) return FALSE; \
      (p)->outlen--;                     \
      *(p)->outbuf++ = id

#define ASN1_WRITE_ID_OCTET(p,id) \
      {                                                  \
         uint32_t val = id;                               \
         if (val > 0x7F) {                               \
            if ((p)->outlen < 1) {                       \
               (p)->errstat = SNMP_ERROR_tooBig;         \
               return FALSE;                             \
            }                                            \
            (p)->outlen--;                               \
            *(p)->outbuf++ = ((val >> 7) & 0x7F) | 0x80; \
            val &= 0x7F;                                 \
         }                                               \
         if ((p)->outlen < 1) {                          \
            (p)->errstat = SNMP_ERROR_tooBig;            \
            return FALSE;                                \
         }                                               \
         (p)->outlen--;                                  \
         *(p)->outbuf++ = val;                           \
      }

#define ASN1_WRITE_ID_INT(p,id) \
      {                                                            \
         uint32_t val = id;                                         \
         if (val > 0x7F) {                                         \
            if (val > 0x3FFF) {                                    \
               if (val > 0x1FFFFF) {                               \
                  if (val > 0xFFFFFFF) {                           \
                     if ((p)->outlen < 1) {                        \
                        (p)->errstat = SNMP_ERROR_tooBig;          \
                        return FALSE;                              \
                     }                                             \
                     (p)->outlen--;                                \
                     *(p)->outbuf++ = ((val >> 28) & 0x7F) | 0x80; \
                     val &= 0xFFFFFFF;                             \
                  }                                                \
                  if ((p)->outlen < 1) {                           \
                     (p)->errstat = SNMP_ERROR_tooBig;             \
                     return FALSE;                                 \
                  }                                                \
                  (p)->outlen--;                                   \
                  *(p)->outbuf++ = (val >> 21) | 0x80;             \
                  val &= 0x1FFFFF;                                 \
               }                                                   \
               if ((p)->outlen < 1) {                              \
                  (p)->errstat = SNMP_ERROR_tooBig;                \
                  return FALSE;                                    \
               }                                                   \
               (p)->outlen--;                                      \
               *(p)->outbuf++ = (val >> 14) | 0x80;                \
               val &= 0x3FFF;                                      \
            }                                                      \
            if ((p)->outlen < 1) {                                 \
               (p)->errstat = SNMP_ERROR_tooBig;                   \
               return FALSE;                                       \
            }                                                      \
            (p)->outlen--;                                         \
            *(p)->outbuf++ = (val >> 7) | 0x80;                    \
            val &= 0x7F;                                           \
         }                                                         \
         if ((p)->outlen < 1) {                                    \
            (p)->errstat = SNMP_ERROR_tooBig;                      \
            return FALSE;                                          \
         }                                                         \
         (p)->outlen--;                                            \
         *(p)->outbuf++ = val;                                     \
      }


/***************************************
**
** Prototypes
**
*/


#endif
/* EOF */
