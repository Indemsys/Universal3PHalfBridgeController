/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   This include file is used to provide constant and structure definitions
*   common for RTC module
*
*
*END************************************************************************/

#ifndef __rtc_h__
#define __rtc_h__ 1

/*--------------------------------------------------------------------------*/
/*
 *                  ERROR CODE DEFINITION
 */

#define RTC_INVALID_TIME        (RTC_ERROR_BASE | 0x01)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rtc_handler {
    INT_ISR_FPTR    isr_func;
    void*           isr_dat;
} RTC_HANDLER, *RTC_HANDLER_PTR;

/*--------------------------------------------------------------------------*/
/*
 *                  PROTOTYPES OF RTC FUNCTIONS
 */

extern int32_t  _rtc_init(void*);
extern int32_t  _rtc_set_time(uint32_t time);
extern int32_t  _rtc_get_time(uint32_t*);
extern int32_t  _rtc_set_alarm(uint32_t, uint32_t);
extern int32_t  _rtc_get_alarm(uint32_t*);
extern int32_t  _rtc_callback_reg(INT_ISR_FPTR, void*);
void            _rtc_isr(void*);

#ifdef __cplusplus
}
#endif

#endif /* __rtc_h__ */

/* EOF */
