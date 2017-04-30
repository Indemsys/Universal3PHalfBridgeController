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
*   The file contains functions prototype, defines, structure
*   definitions private to the fdv Ramdisk drivers
*
*
*END************************************************************************/
#ifndef __adc_prv_h__
#define __adc_prv_h__

/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/

#ifdef __cplusplus
extern "C" {
#endif

#define IO_ERROR_INCOMPLETE_IOCTL       (IO_ERROR_INVALID_IOCTL_CMD) /* this is internal ADC IOCTL and means that IOCTL must be passed from ADC part to trigger part */

/* information stored here concerns only to AD converter */
typedef struct adc_generic {
   uint32_t            resolution;
   uint32_t            adc_frq;       /* frequency of ADC hardware, usually 1 / Tad */
} ADC_GENERIC, * ADC_GENERIC_PTR;

/* information stored here concerns only to AD trigger */
typedef struct adt_generic {
   ADT_TRIGGER_MASK   triggers_fire; /* specifies triggers, which were set and there is still attached channel that did not complete conversion */
   bool            run;           /* specifies if ADC is running (this is OR operation of all channel's ADC_CHANNEL_RUNNING flag) */
} ADT_GENERIC, * ADT_GENERIC_PTR;

typedef struct adc_channel_generic {
   _mqx_uint number;     /* number of channel */
   uint16_t               init_flags;        /* flags copied from initialization structure */
   uint16_t               source;            /* source copied from initialization structure */
   uint32_t               range;             /* output range */
   ADC_RESULT_STRUCT_PTR buffer_start;      /* circular result buffer start pointer */
   ADC_RESULT_STRUCT_PTR buffer_end;        /* circular result buffer end pointer */
   ADC_RESULT_STRUCT_PTR buffer_task;       /* circular result buffer - next read pointer for result */
   ADC_RESULT_STRUCT_PTR buffer_driver;     /* circular result buffer - next write pointer for result */
} ADC_CHANNEL_GENERIC, * ADC_CHANNEL_GENERIC_PTR;

typedef struct adt_channel_generic {
   _mqx_uint number;     /* number of channel */
   uint16_t               init_flags;        /* flags copied from initialization structure */
#define ADC_CHANNEL_RUNNING          (0x01) /* when ADC channel is running, this flag is set */
#define ADC_CHANNEL_RESUMED          (0x02) /* when ADC channel is not paused, this flag is set */
   uint16_t               runtime_flags;     /* generic flags for ADC driver */
   int32_t                offset;            /* offset of time, counter: first this needs to be decremented to zero to start conversion process */
   _mqx_uint             period;            /* time period between 2 samples taken */
   _mqx_uint             time;              /* time to next ADC */
   int32_t                samples;           /* samples, counter: this needs to be decremented to zero to invoke any event */
   int32_t                samples_preset;    /* constant: specifies predefined number of samples */
   ADT_TRIGGER_MASK      trigger;           /* trigger, that triggers this ADC channel */
#if MQX_USE_LWEVENTS
   LWEVENT_STRUCT_PTR    complete_event;    /* pointer to lwevent, triggered after counter_samples reaches zero */
   uint32_t               event_mask;        /* mask of events in complete_event */
#endif
} ADT_CHANNEL_GENERIC, * ADT_CHANNEL_GENERIC_PTR;

typedef uint32_t ADC_HW_CHANNEL_MASK; /* there can be only 32 channels per converter */

/* NOTE: DO NOT MOVE THESE INCLUDES, THEY MUST BE HERE. THEY DEFINES NEW MANDATORY TYPES (E.G. ADC_CHANNEL_BUNDLE) BASED ON GENERIC TYPES */
#if (PSP_MQX_CPU_IS_MCF51QM || PSP_MQX_CPU_IS_MCF51JF || PSP_MQX_CPU_IS_KINETIS)
    #include "adc_kadc_prv.h"
#elif PSP_MQX_CPU_IS_MCF5222X || PSP_MQX_CPU_IS_MCF5223X || PSP_MQX_CPU_IS_MCF5225X
    #include "adc_mcf522xx_prv.h"
#elif PSP_MQX_CPU_IS_MCF51MM
    #include "adc_mcf51mm_prv.h"
#elif PSP_MQX_CPU_IS_MCF51JE
    #include "adc_mcf51je_prv.h"
#elif PSP_MQX_CPU_IS_MCF51AG
    #include "adc_mcf51ag_prv.h"
#elif PSP_MQX_CPU_IS_MCF51EM
    #include "adc_mcf51em_prv.h"
#elif PSP_MQX_CPU_IS_MCF51
    #include "adc_mcf51xx_prv.h"
#elif PSP_MQX_CPU_IS_MCF5441X
    #include "adc_mcf544xx_prv.h"
#else
    #error ADC device driver not supported for processor.
#endif
/* AND NOW, WE HAVE ALL DEFINITIONS FOR DRIVER */

_mqx_int _adc_open(MQX_FILE_PTR, char *, char *);
_mqx_int _adc_close(MQX_FILE_PTR);
_mqx_int _adc_read (MQX_FILE_PTR, char *, _mqx_int);
_mqx_int _adc_write(MQX_FILE_PTR, char *, _mqx_int);
_mqx_int _adc_ioctl(MQX_FILE_PTR, _mqx_uint, void *);

/* these functions must be present for trigger */
_mqx_int _adt_trigger(MQX_FILE_PTR, ADT_CHANNEL_GENERIC_PTR channel, ADT_TRIGGER_MASK mask);
_mqx_int _adt_pause(MQX_FILE_PTR, ADT_CHANNEL_GENERIC_PTR channel, ADT_TRIGGER_MASK mask);
_mqx_int _adt_resume(MQX_FILE_PTR, ADT_CHANNEL_GENERIC_PTR channel, ADT_TRIGGER_MASK mask);
_mqx_int _adt_stop(MQX_FILE_PTR, ADT_CHANNEL_GENERIC_PTR channel, ADT_TRIGGER_MASK mask);
_mqx_int _adt_process_data(ADC_CHANNEL_BUNDLE_PTR channel);

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
