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
*
*   This file contains the type definitions for the Kinetis microcontrollers.
*
*
*END************************************************************************/

#ifndef __kinetis_h__
#define __kinetis_h__

#define __kinetis_h__version "$Version:3.8.26.0$"
#define __kinetis_h__date    "$Date:Sep-21-2012$"
#ifndef __ASM__

/* Include header file for specific Kinetis platform */
#if     (MQX_CPU == PSP_CPU_MK10DX128Z) || \
        (MQX_CPU == PSP_CPU_MK10DX256Z) || \
        (MQX_CPU == PSP_CPU_MK10DN512Z)
    #include "MK10DZ10.h"
#elif   (MQX_CPU == PSP_CPU_MK10F12)
    #include "MK10F12.h"
#elif   (MQX_CPU == PSP_CPU_MK20D50M)
    #include "MK20D5.h"
#elif   (MQX_CPU == PSP_CPU_MK20DX256)
    #include "MK20D7.h"
#elif   (MQX_CPU == PSP_CPU_MK20DX256Z) || \
        (MQX_CPU == PSP_CPU_MK20DN512Z)
    #include "MK20DZ10.h"
#elif   (MQX_CPU == PSP_CPU_MK20F12)
    #include "MK20F12.h"
#elif   (MQX_CPU == PSP_CPU_MK21DN512)
    #include "MK21D5.h"
#elif   (MQX_CPU == PSP_CPU_MK21FN1M)
    #include "MK21F12.h"
#elif   (MQX_CPU == PSP_CPU_MK22FN512)
    #include "MK22F51212.h"
#elif   (MQX_CPU == PSP_CPU_MK24F120M)
    #include "MK24F25612.h"
#elif   (MQX_CPU == PSP_CPU_MK30DX128Z) || \
        (MQX_CPU == PSP_CPU_MK30DX256Z) || \
        (MQX_CPU == PSP_CPU_MK30DN512Z)
    #include "MK30DZ10.h"
#elif   (MQX_CPU == PSP_CPU_MK40DX128Z) || \
        (MQX_CPU == PSP_CPU_MK40DX256Z) || \
        (MQX_CPU == PSP_CPU_MK40DN512Z)
    #include "MK40DZ10.h"
#elif   (MQX_CPU == PSP_CPU_MK40DX128) || \
        (MQX_CPU == PSP_CPU_MK40DX256) || \
        (MQX_CPU == PSP_CPU_MK40D100M)
    #include "MK40D10.h"
#elif   (MQX_CPU == PSP_CPU_MK50DX256Z) || \
        (MQX_CPU == PSP_CPU_MK50DN512Z)
    #include "MK50DZ10.h"
#elif   (MQX_CPU == PSP_CPU_MK51DX256Z) || \
        (MQX_CPU == PSP_CPU_MK51DN256Z) || \
        (MQX_CPU == PSP_CPU_MK51DN512Z)
    #include "MK51DZ10.h"
#elif   (MQX_CPU == PSP_CPU_MK52DN512Z)
    #include "MK52DZ10.h"
#elif   (MQX_CPU == PSP_CPU_MK53DX256Z) || \
        (MQX_CPU == PSP_CPU_MK53DN512Z)
    #include "MK53DZ10.h"
#elif   (MQX_CPU == PSP_CPU_MK60DX256Z) || \
        (MQX_CPU == PSP_CPU_MK60DN256Z) || \
        (MQX_CPU == PSP_CPU_MK60DN512Z)
    #include "MK60DZ10.h"
#elif   (MQX_CPU == PSP_CPU_MK60D100M)
    #include "MK60D10.h"
#elif   (MQX_CPU == PSP_CPU_MK60DF120M)
    #include "MK60F12.h"
#elif   (MQX_CPU == PSP_CPU_MK64F120M)
    #include "MK64F12.h"
#elif   (MQX_CPU == PSP_CPU_MK65F180M)
    #include "MK65F18.h"
#elif   (MQX_CPU == PSP_CPU_MK70F120M)
    #include "MK70F12.h"
#elif   (MQX_CPU == PSP_CPU_MK70F150M)
    #include "MK70F15.h"
#else
    #error "Wrong CPU definition"
#endif

    #include "nvic.h"
    #include "kinetis_mpu.h"

#endif /* __ASM__ */

#ifdef __cplusplus
extern "C" {
#endif


/*
*******************************************************************************
**
**                  CONSTANT DEFINITIONS
**
*******************************************************************************
*/

#if   (MQX_CPU == PSP_CPU_MK70F120M)
    #if !MQX_DISABLE_CACHE
        #define PSP_HAS_CODE_CACHE                      1
        #define PSP_HAS_DATA_CACHE                      1
        #define PSP_BYPASS_P3_WFI                       1
    #endif // MQX_DISABLE_CACHE
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1
    /* enable FPU for this platform */
    #define PSP_HAS_FPU                                 1

#elif   (MQX_CPU == PSP_CPU_MK70F150M)
    #if !MQX_DISABLE_CACHE
        #define PSP_HAS_CODE_CACHE                      1
        #define PSP_HAS_DATA_CACHE                      1
        #define PSP_BYPASS_P3_WFI                       1
    #endif // MQX_DISABLE_CACHE
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1
    /* enable FPU for this platform */
    #define PSP_HAS_FPU                                 1

#elif   (MQX_CPU == PSP_CPU_MK60DF120M)
    #if !MQX_DISABLE_CACHE
        #define PSP_HAS_CODE_CACHE                      1
        #define PSP_HAS_DATA_CACHE                      1
        #define PSP_BYPASS_P3_WFI                       1
    #endif // MQX_DISABLE_CACHE
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1
    /* enable FPU for this platform */
    #define PSP_HAS_FPU                                 1
#elif   (MQX_CPU == PSP_CPU_MK65F180M)
    #if !MQX_DISABLE_CACHE
        #define PSP_HAS_CODE_CACHE                      0 /// will enable later
        #define PSP_HAS_DATA_CACHE                      0
        #define PSP_BYPASS_P3_WFI                       1
    #endif // MQX_DISABLE_CACHE
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1
    /* enable FPU for this platform */
    #define PSP_HAS_FPU                                 1
#elif   (MQX_CPU == PSP_CPU_MK10DN512Z)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK20D50M)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK20DN512Z)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK20F12)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK21DN512)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK21FN1M)
    /* enable FPU for this platform */
    #define PSP_HAS_FPU                                 1

    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK22FN512)
    /* enable FPU for this platform */
    #define PSP_HAS_FPU                                 1

#elif   (MQX_CPU == PSP_CPU_MK22FN256)
    /* enable FPU for this platform */
    #define PSP_HAS_FPU                                 1
    
#elif   (MQX_CPU == PSP_CPU_MK22FN128)
    /* enable FPU for this platform */
    #define PSP_HAS_FPU                                 1

#elif   (MQX_CPU == PSP_CPU_MK24F120M)
    /* enable FPU for this platform */
    #define PSP_HAS_FPU                                 1

#elif   (MQX_CPU == PSP_CPU_MK30DN512Z)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK40DN512Z)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK40D100M)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK50DN512Z)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK51DN256Z)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK51DN512Z)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK52DN512Z)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK53DN512Z)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK60DN256Z)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK60DN512Z)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK60D100M)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1

#elif   (MQX_CPU == PSP_CPU_MK64F120M)
    /* enable flash swap for this platform */
    #define PSP_HAS_FLASH_SWAP                          1
    /* enable FPU for this platform */
    #define PSP_HAS_FPU                                 1
#endif

/* flash swap default value */
#ifndef PSP_HAS_FLASH_SWAP
#define PSP_HAS_FLASH_SWAP                      0
#endif

/* Cache and MMU definition values */
#ifndef PSP_HAS_MMU
#define PSP_HAS_MMU                             0
#endif

#ifndef PSP_HAS_CODE_CACHE
#define PSP_HAS_CODE_CACHE                      0
#endif

#ifndef PSP_HAS_DATA_CACHE
#define PSP_HAS_DATA_CACHE                      0
#endif

#ifndef PSP_HAS_FPU
#define PSP_HAS_FPU                             0
#endif

#ifndef PSP_BYPASS_P3_WFI
#define PSP_BYPASS_P3_WFI                       0
#endif

#define PSP_CACHE_LINE_SIZE                     (0x10)

#ifndef __ASM__

/* Standard cache macros */
#if PSP_HAS_DATA_CACHE
#define _DCACHE_ENABLE(n)               _dcache_enable()
#define _DCACHE_DISABLE()               _dcache_disable()
#define _DCACHE_FLUSH()                 _dcache_flush()
#define _DCACHE_FLUSH_LINE(p)           _dcache_flush_line(p)
#define _DCACHE_FLUSH_MLINES(p, m)      _dcache_flush_mlines(p,m)
#define _DCACHE_INVALIDATE()            _dcache_invalidate()
#define _DCACHE_INVALIDATE_LINE(p)      _dcache_invalidate_line(p)
#define _DCACHE_INVALIDATE_MLINES(p, m) _dcache_invalidate_mlines(p, m)

#define _DCACHE_FLUSH_MBYTES(p, m)             _DCACHE_FLUSH_MLINES(p, m)
#define _DCACHE_INVALIDATE_MBYTES(p, m)        _DCACHE_INVALIDATE_MLINES(p, m)

#else
#define _DCACHE_ENABLE(n)
#define _DCACHE_DISABLE()
#define _DCACHE_FLUSH()
#define _DCACHE_FLUSH_LINE(p)
#define _DCACHE_FLUSH_MLINES(p, m)
#define _DCACHE_INVALIDATE()
#define _DCACHE_INVALIDATE_LINE(p)
#define _DCACHE_INVALIDATE_MLINES(p, m)

#define _DCACHE_FLUSH_MBYTES(p, m)
#define _DCACHE_INVALIDATE_MBYTES(p, m)
#endif

#if PSP_HAS_CODE_CACHE
#define _ICACHE_ENABLE(n)                   _icache_enable()
#define _ICACHE_DISABLE()                   _icache_disable()
#define _ICACHE_INVALIDATE()                _icache_invalidate()
#define _ICACHE_INVALIDATE_LINE(p)          _icache_invalidate_line(p)
#define _ICACHE_INVALIDATE_MLINES(p, m)     _icache_invalidate_mlines(p, m)
#else
#define _ICACHE_ENABLE(n)
#define _ICACHE_DISABLE()
#define _ICACHE_FLUSH()
#define _ICACHE_FLUSH_LINE(p)
#define _ICACHE_FLUSH_MLINES(p, m)
#define _ICACHE_INVALIDATE()
#define _ICACHE_INVALIDATE_LINE(p)
#define _ICACHE_INVALIDATE_MLINES(p, m)
#endif

/** Sleep function definition.
 */
#if PSP_BYPASS_P3_WFI
#define _ASM_SLEEP(param)    {extern void(*_sleep_p3_ram)(uint32_t*);_sleep_p3_ram(param);}
#else
#define _ASM_SLEEP(param)    _ASM_WFI()
#endif

#define PSP_INTERRUPT_TABLE_INDEX               IRQInterruptIndex

/*
*******************************************************************************
**
**                  TYPE DEFINITIONS
**
*******************************************************************************
*/



/*
*******************************************************************************
**
**              FUNCTION PROTOTYPES AND GLOBAL EXTERNS
**
*******************************************************************************
*/

#define _psp_mem_check_access(addr, size, flags)    \
                    _kinetis_mpu_sw_check(addr, size, flags)

#define _psp_mem_check_access_mask(addr, size, flags, mask) \
                    _kinetis_mpu_sw_check_mask(addr, size, flags, mask)

#define _psp_mpu_add_region(start, end, flags)  \
                    _kinetis_mpu_add_region(start, end, flags)

/* PSP Cache prototypes */
void _dcache_enable(void);
void _dcache_disable(void);
void _dcache_flush(void);
void _dcache_flush_line(void *);
void _dcache_flush_mlines(void *, uint32_t);
void _dcache_invalidate(void);
void _dcache_invalidate_line(void *);
void _dcache_invalidate_mlines(void *, uint32_t);

void _icache_enable(void);
void _icache_disable(void);
void _icache_flush(void);
void _icache_flush_line(void *);
void _icache_flush_mlines(void *, uint32_t);
void _icache_invalidate(void);
void _icache_invalidate_line(void *);
void _icache_invalidate_mlines(void *, uint32_t);

#endif /* __ASM__ */

#ifdef __cplusplus
}
#endif

#endif /* __kinetis_h__ */
