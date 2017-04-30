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
*   required for the TCHRES driver
*
*
*END************************************************************************/
#ifndef __tchres_h__
#define __tchres_h__

#ifdef __cplusplus
extern "C" {
#endif

#define IO_IOCTL_TCHSCR_GET_POSITION     _IO(IO_TYPE_TCHRES,0x01)
#define IO_IOCTL_TCHSCR_GET_POSITION_RAW _IO(IO_TYPE_TCHRES,0x02)
#define IO_IOCTL_TCHSCR_SET_CALIBRATION  _IO(IO_TYPE_TCHRES,0x03)
#define IO_IOCTL_TCHSCR_GET_CALIBRATION  _IO(IO_TYPE_TCHRES,0x04)
#define IO_IOCTL_TCHSCR_GET_RAW_LIMITS   _IO(IO_TYPE_TCHRES,0x05)

#define TCHRES_OK                      (0x00)
#define TCHRES_ERROR_INVALID_PARAMETER (TCHRES_ERROR_BASE | 0x01)
#define TCHRES_ERROR_NO_TOUCH          (TCHRES_ERROR_BASE | 0x02)
#define TCHRES_ERROR_TIMEOUT           (TCHRES_ERROR_BASE | 0x03)

/* adc event macros */
#define TCH_ADC_X_COMPLETE  0x01
#define TCH_ADC_Y_COMPLETE  0x02

/* ADC channels settings macros - change, if needed */
#define TCHRES_ADC_FLAGS         (ADC_CHANNEL_MEASURE_ONCE | ADC_CHANNEL_START_TRIGGERED)
#define TCHRES_ADC_NUM_SAMPLES   (1)
#define TCHRES_ADC_TIME_PERIOD   (250)
#define TCHRES_ADC_RANGE         (0x10000)
#define TCHRES_ADC_BUFFER_SIZE   (5)

typedef enum
{
   TWO_POINT_CALIB
}  TCHRES_CALIB_METHOD;

/* pin function structure */
typedef struct tchres_pin_funct_struct
{
   uint32_t        X_PLUS_GPIO_FUNCTION;
   uint32_t        X_PLUS_ADC_FUNCTION;
   uint32_t        Y_PLUS_GPIO_FUNCTION;
   uint32_t        Y_PLUS_ADC_FUNCTION;
   uint32_t        X_MINUS_GPIO_FUNCTION;
   uint32_t        Y_MINUS_GPIO_FUNCTION;
} TCHRES_PIN_FUNCT_STRUCT, * TCHRES_PIN_FUNCT_STRUCT_PTR;

/* pin connection structure, defines the gpio pin for touch screen interface */
typedef struct tchres_pin_config_struct
{
   /* BSP_X_PLUS… must be defined in device.h in gpio driver naming standard */
   /* pin configuration */
   LWGPIO_PIN_ID       X_PLUS;    /* x + gpio pin definition */
   LWGPIO_PIN_ID       X_MINUS;   /* x - gpio pin definition */
   LWGPIO_PIN_ID       Y_PLUS;    /* y + gpio pin definition */
   LWGPIO_PIN_ID       Y_MINUS;   /* y - gpio pin definition */
   /* gpio and adc pin mux masks */
   TCHRES_PIN_FUNCT_STRUCT PIN_FUNCT;

} TCHRES_PIN_CONFIG_STRUCT, * TCHRES_PIN_CONFIG_STRUCT_PTR;

/* install parameters - adc devices used for measuring on X+ and Y+ electrodes */
typedef struct tchres_install_param_struct
{
	char *ADC_XPLUS_DEVICE; /* ADC device for X+ electrode */
	char *ADC_YPLUS_DEVICE; /* ADC device for Y+ electrode */
} TCHRES_INSTALL_PARAM_STRUCT, * TCHRES_INSTALL_PARAM_STRUCT_PTR;

/* result structure */
typedef struct tchres_position
{
    int16_t  X;  /* X position*/
    int16_t  Y; /* Y position*/
} TCHRES_POSITION_STRUCT, * TCHRES_POSITION_STRUCT_PTR;

/* Adc limits struct */
typedef struct tchres_adc_limits
{
    uint16_t   FULL_SCALE;
    int16_t    X_TOUCH_MIN;
    int16_t    Y_TOUCH_MIN;
    int16_t    X_TOUCH_MAX;
    int16_t    Y_TOUCH_MAX;
} TCHRES_ADC_LIMITS_STRUCT, * TCHRES_ADC_LIMITS_STRUCT_PTR;

/* Init structure */
typedef struct tchres_init_struct
{
   /* structure with pins connected to touch_screen detection */
   TCHRES_PIN_CONFIG_STRUCT   PIN_CONFIG;

   /* Touch screen adc limits structure */
   TCHRES_ADC_LIMITS_STRUCT   ADC_LIMITS;

   /* Touch screen adc channel X setting structure */
   LWGPIO_PIN_ID              ADC_CHANNEL_X_SOURCE;
   ADT_TRIGGER_MASK           ADC_CHANNEL_X_TRIGGER;

   /* Touch screen adc channel Y setting structure */
   LWGPIO_PIN_ID              ADC_CHANNEL_Y_SOURCE;
   ADT_TRIGGER_MASK           ADC_CHANNEL_Y_TRIGGER;

#if 0 /* for later use */
   /* Touch Screen calibration information */
   TCHRES_CALIB_METHOD CALIBRATION_METHOD;
#endif
} TCHRES_INIT_STRUCT, * TCHRES_INIT_STRUCT_PTR;


/* functions */
_mqx_uint _io_tchres_install( char *identifier, TCHRES_INIT_STRUCT_PTR  init_ptr, TCHRES_INSTALL_PARAM_STRUCT_PTR install_params);

#if 0 /* for later use */
/* Calibration struct :
**  this structure defines coordinates for two calibration points upper left
** corner and lower right corner in pixels. Resistive internally recalculate
** this coordinates to ADC values after Ioctl_Get_calibrate. */
typedef struct tchres_callibration_struct
{
   /* [X0, Y0] calibration coordinates in pixels */
   int16_t   X_CALIB_POS;
   int16_t   Y_CALIB_POS;
   int16_t   X_CALIB_RAW;
   int16_t   Y_CALIB_RAW;
} TCHRES_CALLIBRATION_STRUCT, * TCHRES_CALLIBRATION_STRUCT_PTR;
#endif

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
