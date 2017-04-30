
/*HEADER**********************************************************************
*
* Copyright 2010 Freescale Semiconductor, Inc.
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
*   This file contains the support functions for byte/word and others
*   manipulations.
*
*
*END************************************************************************/

#include "mqx_inc.h"

#ifdef __CODEWARRIOR__

asm uint16_t _psp_swap2byte(uint16_t n) {
    rev16 r0, r0
    bx lr
}

asm uint32_t _psp_swap4byte(uint32_t n) {
    rev r0, r0
    bx lr
}

asm void __set_BASEPRI(uint32_t basePri) {
    msr basepri, r0
    bx lr
}

#elif defined(__GNUC__) /* gcc */


/*!
 * \brief Swap 2 bytes
 * 
 * \param n 
 *
 * \return uint16_t
 */
uint16_t _psp_swap2byte(uint16_t n)
{
    uint16_t ret;

    __asm volatile ("rev16 %0, %1" :"=r" (ret):"r" (n));
    return(ret);
}

/*!
 * \brief Swap 4 bytes
 * 
 * \param n 
 *
 * \return uint32_t
 */
uint32_t _psp_swap4byte(uint32_t n)
{
    uint32_t ret;

    __asm volatile ("rev %0, %1" :"=r" (ret):"r" (n));
    return(ret);
}

#elif defined(__CC_ARM) /* Keil, DS5 */

/*!
 * \brief
 * 
 * \param basePri
 */
#if PSP_MQX_CPU_IS_ARM_CORTEX_M4
__asm void __set_BASEPRI(uint32_t basePri)
{
    msr basepri, r0
    bx lr
}
#endif

/*!
 * \brief Swap 2 bytes
 * 
 * \param n 
 *
 * \return uint16_t
 */
uint16_t _psp_swap2byte(uint16_t n)
{
    uint16_t ret;

    __asm volatile ("rev16 ret, n");
    return(ret);
}

/*!
 * \brief Swap 4 bytes
 * 
 * \param n 
 *
 * \return uint32_t
 */
uint32_t _psp_swap4byte(uint32_t n)
{
    uint32_t ret;

    __asm volatile ("rev ret, n");
    return(ret);
}

#elif defined(__ICCARM__)

/* IAR EWARM library contains all needed code */

#else /* __CODEWARRIOR__, __ICCARM__ */

/*!
 * \brief Swap 2 bytes
 * 
 * \param n
 *
 * \return uint16_t
 */
uint16_t _psp_swap2byte(uint16_t n)
{ /* Body */
    unsigned char temp[2];

    temp[1] = *((unsigned char *)&n);
    temp[0] = ((unsigned char *)&n)[1];
    return (*(uint16_t *)temp);
} /* Endbody */

/*!
 * \brief Swap 4 bytes
 * 
 * \param n 
 *
 * \return uint32_t
 */
uint32_t _psp_swap4byte(uint32_t n)
{ /* Body */
    unsigned char temp[4];

    temp[3] = *((unsigned char *)&n);
    temp[2] = ((unsigned char *)&n)[1];
    temp[1] = ((unsigned char *)&n)[2];
    temp[0] = ((unsigned char *)&n)[3];
    return (*(uint32_t *)temp);
} /* Endbody */

#endif /* __CODE_WARRIOR__, __ICCARM__ */
