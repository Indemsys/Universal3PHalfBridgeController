/*HEADER**********************************************************************
*
* Copyright 2009 Freescale Semiconductor, Inc.
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
*   This file contains the definitions of constants and structures
*   required for the SPI driver
*
*
*END************************************************************************/
#ifndef __tchres_prv_h__
#define __tchres_prv_h__

#ifdef __cplusplus
extern "C" {
#endif

#if 0
/* for later use */
typedef struct tchres_calib_struct
{
  uint16_t   X_RESOLUTION;
  uint16_t   Y_RESOLUTION;
  int16_t    K_X;
  int16_t    Q_X;
  int16_t    K_Y;
  int16_t    Q_Y;
} TCHRES_CALIB_STRUCT, * TCHRES_CALIB_STRUCT_PTR ;
#endif

typedef struct tchres_struct
{
    /* pin information */
    LWGPIO_STRUCT X_PLUS_PIN;
    LWGPIO_STRUCT Y_PLUS_PIN;
    LWGPIO_STRUCT X_MINUS_PIN;
    LWGPIO_STRUCT Y_MINUS_PIN;

    /* every pin can have another mux mask */
    TCHRES_PIN_FUNCT_STRUCT PIN_FUNCT;

   /* adc limits structure */
   TCHRES_ADC_LIMITS_STRUCT   ADC_LIMITS;
   /* open file counter */
   unsigned char    COUNT;

   /* ADC device info */
   char   *ADC_XPLUS_DEVICE;
   char   *ADC_YPLUS_DEVICE;
   MQX_FILE_PTR   ADC_CHANNEL_X;
   MQX_FILE_PTR   ADC_CHANNEL_Y;
   ADC_INIT_CHANNEL_STRUCT_PTR   ADC_CHANNEL_X_STRUCT;
   ADC_INIT_CHANNEL_STRUCT_PTR   ADC_CHANNEL_Y_STRUCT;
   LWEVENT_STRUCT TCH_EVENT;

#if 0
    /* callibration data */
    unsigned char    NUMBER_OF_CALIB_POINTS;
    /* pointer to TCHRES_CALLIBRATION_STRUCT array */
   TCHRES_CALLIBRATION_STRUCT_PTR  CALLIB_POINTS_STRUCT_PTR;
   /* calibration data */
   TCHRES_CALIB_METHOD CALIBRATION_METHOD;
   TCHRES_CALIB_STRUCT        CALIB_DATA;
#endif

} TCHRES_DEVICE_STRUCT, * TCHRES_DEVICE_STRUCT_PTR;

typedef enum
{
   X_PLUS,
   X_MINUS,
   Y_PLUS,
   Y_MINUS
} TCHRES_PIN_SELECT_ENUM, * TCHRES_PIN_SELECT_ENUM_PTR ;

typedef enum
{
    FULL_SCALE,
    X_TOUCH_MIN,
    Y_TOUCH_MIN,
    X_TOUCH_MAX,
    Y_TOUCH_MAX
} TCHRES_LIMITS_ENUM, * TCHRES_LIMITS_ENUM_PTR;

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
