#ifndef __adc_conf_h__
#define __adc_conf_h__
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
*   ADC driver configuration header file
*
*
*END************************************************************************/

/* Note that these are SW channels for MQX driver, not related to the HW */
#define ADC_MAX_SW_CHANNELS       (2)
/* ADC driver can hold file name in the driver to prevent open file 2 times */
#define ADC_STORE_FILENAMES       1

#endif

/* EOF */
