
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains definitions required for floating point 
*   formatted I/O.
*
*
*END************************************************************************/
#ifndef __fpio_prv_h__
#define __fpio_prv_h__

/*----------------------------------------------------------------------*/
/*
 *                          CONSTANT DEFINITIONS
 */

/* Floating point Default PRECision Value.  */
#define FDPRECV         (6)

/* Number of decimal digits to the left of an e format output. */
#define LEFTOFE         (1)

/* 
 * Number of decimal digits to the left 
 * of an f format output.  This is 
 * presumably a reasonable maximum.   
 */
#define LEFTOFF         (308)

/* 
 * IEEE 754 double-precision binary floating-point format
 * The maximum double number value is 1.797693E+308, so it need 308 chars to represent it
 * and 1 char to add zero-ending string space   
 */

#define NDIG            (309)
#define MAX_PRECISION   (17)

#define FP_WORDSIZE     (16)
#define FP_NUMWORDS     (4)
#define FP_FLT          (0)
#define FP_DBL          (1)
#define BIAS            (1023)
#define MAX_EXP         (2046)
#define MIN_EXP         (-51)
#define FMAX_EXP        (254)
#define FMIN_EXP        (-22)

#ifndef DBL_MAX_10_EXP
#define DBL_MAX_10_EXP  (308)
#endif

#ifndef HUGE_VAL
#define HUGE_VAL         99.e999
#endif

#if PSP_ENDIAN == MQX_LITTLE_ENDIAN
#define HIGH              (1)
#define LOW               (0)
#else
#define HIGH              (0)
#define LOW               (1)
#endif

#define BYTES_PER_DOUBLE  (8)
#define MANT_BITS         (52)
#define NON_MANT_BITS     (12)
#define EXP_BIAS          (1023)

/*--------------------------------------------------------------------------*/
/*
 *                            MACRO DEFINTIONS
 */

#define EXP_MASK        (0x7ff00000L)
#define FEXP_MASK       (0x7f800000L)

#define SIGN_MASK       (0x80000000L)

#define MAXEXP_MASK     (0x000007ffL)
#define FMAXEXP_MASK    (0x000000ffL)

#define MANT_MASK       (0x000fffffL)
#define FMANT_MASK      (0x007fffffL)

#define HIDDEN_MASK     (0x00100000L)
#define FHIDDEN_MASK    (0x00800000L)

/* macros for a, where a is the address of 64 bits of ieee double */
#define EXPOF(a)        (((a)[HIGH] & EXP_MASK) >> 20)
#define FEXPOF(a)       (((a) & FEXP_MASK) >> 23)

#define SIGNOF(a)       (((a)[HIGH] & SIGN_MASK) != 0)
#define FSIGNOF(a)      (((a) & SIGN_MASK) != 0)

#define ISZERO(a)       ( !( ((a)[HIGH] & ~SIGN_MASK) | ((a)[LOW])) )
#define FISZERO(a)      (((a) & 0x7fffffffL) == 0)

#define MANTZERO(a)     ( !( ((a)[HIGH] &  MANT_MASK) | ((a)[LOW])) )
#define FMANTZERO(a)    (((a) &  FMANT_MASK) == 0)

#define ISINFINITY(a)   ((EXPOF(a) == MAXEXP_MASK) &&  MANTZERO(a))
#define FISINFINITY(a)  ((FEXPOF(a) == FMAXEXP_MASK) &&  FMANTZERO(a))

#define ISNAN(a)        ((EXPOF(a) == MAXEXP_MASK) && !(MANTZERO(a)))
#define FISNAN(a)       ((FEXPOF(a) == FMAXEXP_MASK) && !(FMANTZERO(a)))


/*--------------------------------------------------------------------------*/
/*
 *                            DATATYPE DECLARATIONS
 */

/*!  
 * \cond DOXYGEN_PRIVATE
 *  
 * /brief This union is used so that we can access the fp number as bytes, 
 * longwords, or as a floating point number.    
 */
typedef union fp_union {
    /*! \brief Double data type. */
    double      DOUBLE;
    /*! \brief Long data type. */
    uint32_t     LONG[2];
    /*! \brief Byte data type. */
    uint8_t      BYTE[8];
} FP_UNION, * FP_UNION_PTR;
/*! \endcond */

/*--------------------------------------------------------------------------*/
/*
 *                            FUNCTION PROTOTYPES
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern char      *_io_cvt(double, int32_t, int32_t *, char *, char, char *,char type);
extern int32_t     _io_dtoa(int32_t, char *);
extern int32_t     _io_dtoe(char *, double, char *, char, char, char, int32_t,
   char);        
extern int32_t     _io_dtof(char *, double, char *, char, char, char, int32_t,
   char);        
extern int32_t     _io_dtog(char *, double, char *, char, char, char, int32_t,
   char);        
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
