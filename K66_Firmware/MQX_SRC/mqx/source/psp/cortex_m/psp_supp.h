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
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*   This file contains the support functions for byte/word and others
*   manipulations.
*
*
*END************************************************************************/
#ifndef __PSP_SUPP_H__
#define __PSP_SUPP_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __ASM__

#if defined(__CODEWARRIOR__)

uint16_t _psp_swap2byte(uint16_t n);
uint32_t _psp_swap4byte(uint32_t n);
void __set_BASEPRI(uint32_t basePri);

#define _PSP_SWAP2BYTE(n)   _psp_swap2byte(n)
#define _PSP_SWAP4BYTE(n)   _psp_swap4byte(n)

#elif defined(__ICCARM__) /* IAR */

#define _PSP_SWAP2BYTE(n)   (uint16_t)__REVSH(n)
#define _PSP_SWAP4BYTE(n)   __REV(n)

#elif defined(__CC_ARM) /* Keil, DS5 */

uint16_t _psp_swap2byte(uint16_t n);
uint32_t _psp_swap4byte(uint32_t n);

#define _PSP_SWAP2BYTE(n)   _psp_swap2byte(n)
#define _PSP_SWAP4BYTE(n)   _psp_swap4byte(n)

#else

uint16_t _psp_swap2byte(uint16_t n);
uint32_t _psp_swap4byte(uint32_t n);

#define _PSP_SWAP2BYTE(n)   _psp_swap2byte(n)
#define _PSP_SWAP4BYTE(n)   _psp_swap4byte(n)

#endif //__CODEWARRIOR__, __ICCARM__

#endif // __ASM__

#ifdef __cplusplus
}
#endif

#endif
