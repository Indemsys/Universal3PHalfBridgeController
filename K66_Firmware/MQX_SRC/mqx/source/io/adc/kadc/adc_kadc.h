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
*   CPU specific ADC driver header file
*
*
*END************************************************************************/

#ifndef __adc_kadc_h__
#define __adc_kadc_h__

/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/

#ifdef __cplusplus
extern "C" {
#endif

/* NOTE:
** For 13 bit differential mode use ADC_RESOLUTION_12BIT
** For 11 bit differential mode use ADC_RESOLUTION_10BIT
** For 9 bit differential mode use ADC_RESOLUTION_8BIT
*/
#define ADC_RESOLUTION_16BIT    ADC_CFG1_MODE(0x03)
#define ADC_RESOLUTION_12BIT    ADC_CFG1_MODE(0x01)
#define ADC_RESOLUTION_10BIT    ADC_CFG1_MODE(0x02)
#define ADC_RESOLUTION_8BIT     ADC_CFG1_MODE(0x00)
#define ADC_RESOLUTION_DEFAULT (ADC_RESOLUTION_16BIT)

#define ADC_SOURCE_CHANNEL(x)   (x)
#define ADC_SOURCE_MUXSEL_SHIFT (9)
#define ADC_SOURCE_MUXSEL_X     (0x0000) /* none of A / B is forced to use */
#define ADC_SOURCE_MUXSEL_A     (0x0200) /* the A pair is forced to use */
#define ADC_SOURCE_MUXSEL_B     (0x0400) /* the B pair is forced to use */
#define ADC_SOURCE_DIFF         (0x0800)
#define ADC_SOURCE_MODULE_SHIFT (12)
#define ADC_SOURCE_MODULE(x)    (x << ADC_SOURCE_MODULE_SHIFT)
#define ADC_SOURCE_MODULE_ANY   ADC_SOURCE_MODULE(0x00)

#define ADC_GET_CHANNEL(x)  ((x) & 0x001F)
#define ADC_GET_MUXSEL(x)   ((x) & 0x0600)
#define ADC_GET_DIFF(x)     ((x) & 0x0800)
#define ADC_GET_MODULE(x)   ((x) & 0xF000)


#define ADC_PDB_TRIGGER           (0x10000) /* HW specific trigger(s) must be controlled by hardware */

/*
**  IOCTL ADC16 hardware specific commands used in driver
*/
#define ADC_IOCTL_CALIBRATE          _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 0)
#define ADC_IOCTL_SET_CALIBRATION    _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 1)
#define ADC_IOCTL_GET_CALIBRATION    _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 2)
#define ADC_IOCTL_SET_LONG_SAMPLE    _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 3)
#define ADC_IOCTL_SET_SHORT_SAMPLE   _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 4)
#define ADC_IOCTL_SET_HIGH_SPEED     _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 5)
#define ADC_IOCTL_SET_LOW_SPEED      _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 6)
#define ADC_IOCTL_SET_HW_AVERAGING   _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 7)
#define ADC_IOCTL_SET_IDELAY         _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 8)
#define ADC_IOCTL_SET_IDELAYREG      _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 9)
#define ADC_IOCTL_SET_IDELAY_FCN     _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 10)
#define ADC_IOCTL_SET_ERROR_FCN      _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 11)
#define ADC_IOCTL_SET_IDELAY_PROCESS _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 12)
#define ADC_IOCTL_SET_INT_PROCESS    _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 13)
#define ADC_IOCTL_SET_BASE_PERIOD    _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 14)
#define ADC_IOCTL_TRIM_BASE_PERIOD   _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 15)
#define ADC_IOCTL_SET_OFFSET         _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 16)
#define ADC_IOCTL_SET_PLUS_GAIN      _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 17)
#define ADC_IOCTL_SET_MINUS_GAIN     _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 18)
#define ADC_IOCTL_SET_THOLD          _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 19)
#define ADC_IOCTL_SET_THOLD_FCN      _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 20)
#define ADC_IOCTL_SET_DELAYREG       _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 21)
#define ADC_IOCTL_SET_TRIGGER        _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 22)
#define ADC_IOCTL_SET_REFERENCE      _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 23)
#define ADC_IOCTL_SET_PGA_GAIN       _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 24)
#define ADC_IOCTL_GET_PGA_GAIN       _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 25)
#define ADC_IOCTL_ENABLE_CHOPPING    _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 26)
#define ADC_IOCTL_DISABLE_CHOPPING   _IO(IO_TYPE_ADC, ADC_IOCTL_HW_SPECIFIC + 27)

/*
**  ADC16 hardware specific errors
*/
#define ADC_ERROR_PERIOD             (ADC_ERROR_BASE | (ADC_ERROR_HW_SPECIFIC + 0x00)) /* cannot run if basic period was not set */
#define ADC_ERROR_HWTRIGGER          (ADC_ERROR_BASE | (ADC_ERROR_HW_SPECIFIC + 0x01)) /* only HW trigger is supported */
#define ADC_ERROR_NOT_SUPPORTED      (ADC_ERROR_BASE | (ADC_ERROR_HW_SPECIFIC + 0x02)) /* the operation is not supported because of HW limitations */

/*
** kadc16_calib_struct
** Structure used to store ADC16 calibration data
*/
typedef struct kadc_calib_struct
{
    uint16_t OFS;     /*** Offset Correction Register                        ***/
    uint16_t PG;      /*** Plus-Side Gain Register                           ***/
#if ADC_HAS_DIFFERENTIAL_CHANNELS
    uint16_t MG;      /*** Minus-Side Gain Register                          ***/
#endif /* ADC_HAS_DIFFERENTIAL_CHANNELS */
    unsigned char   CLPD;    /*** Plus-Side General Calibration Value Register D    ***/
    unsigned char   CLPS;    /*** Plus-Side General Calibration Value Register S    ***/
    uint16_t CLP4;    /*** Plus-Side General Calibration Value Register 4    ***/
    uint16_t CLP3;    /*** Plus-Side General Calibration Value Register 3    ***/
    unsigned char   CLP2;    /*** Plus-Side General Calibration Value Register 2    ***/
    unsigned char   CLP1;    /*** Plus-Side General Calibration Value Register 1    ***/
    unsigned char   CLP0;    /*** Plus-Side General Calibration Value Register 0    ***/
#if ADC_HAS_DIFFERENTIAL_CHANNELS
    unsigned char   CLMD;    /*** Minus-Side General Calibration Value Register D   ***/
    unsigned char   CLMS;    /*** Minus-Side General Calibration Value Register S   ***/
    uint16_t CLM4;    /*** Minus-Side General Calibration Value Register 4   ***/
    uint16_t CLM3;    /*** Minus-Side General Calibration Value Register 3   ***/
    unsigned char   CLM2;    /*** Minus-Side General Calibration Value Register 2   ***/
    unsigned char   CLM1;    /*** Minus-Side General Calibration Value Register 1   ***/
    unsigned char   CLM0;    /*** Minus-Side General Calibration Value Register 0   ***/
#endif /* ADC_HAS_DIFFERENTIAL_CHANNELS */
} KADC_CALIB_STRUCT;
typedef volatile struct kadc_calib_struct * KADC_CALIB_STRUCT_PTR;

typedef enum {
    ADC_CLK_BUSCLK,     /* selects BUSCLK */
    ADC_CLK_BUSCLK2,    /* selects BUSCLK/2 */
    ADC_CLK_BUSCLK_ANY, /* selects BUSCLK or BUSCLK/2 */
    ADC_CLK_ALTERNATE,
    ADC_CLK_ASYNC
} ADC_CLOCK_SOURCE;

typedef enum {
    ADC_DIV_1 = 0,
    ADC_DIV_2 = 1,
    ADC_DIV_4 = 2,
    ADC_DIV_8 = 3,
    ADC_DIV_ANY = -1 /* selects automatically the fastest one */
} ADC_CLOCK_DIV;

typedef enum {
    ADC_LPC_LOW =    ADC_CFG1_ADLPC_MASK, /* use low power of ADC (reduces max. ADCCLK frequency) */
    ADC_LPC_NORMAL = 0                    /* use normal power of ADC */
} ADC_LPC;

typedef enum {
    ADC_HSC_NORMAL = ADC_CFG2_ADHSC_MASK, /* use normal speed */
    ADC_HSC_HIGH = 0                      /* use high speed */
} ADC_HSC;

typedef enum {
    ADC_PDB_COMP1OUT = 0,   /* Comparator 1 output trigger */
    ADC_PDB_COMP2OUT = 4,   /* Comparator 2 output trigger */
    ADC_PDB_EXTRIG   = 16,  /* External event trigger */
    ADC_PDB_SWTRIG   = 28   /* Software trigger */
} ADC_PDB_TRIGSEL;

typedef enum {
    ADC_REF_VREF, /* internal voltage reference */
    ADC_REF_VALT, /* alternate voltage reference */
    ADC_REF_VBG   /* internal bandgap Vbgh and Vbgl */
} ADC_REFERENCE;

#if ADC_HAS_PGA
typedef enum {
    ADC_PGA_GAIN_1   = ADC_PGA_PGAG(0),
    ADC_PGA_GAIN_2   = ADC_PGA_PGAG(1),
    ADC_PGA_GAIN_4   = ADC_PGA_PGAG(2),
    ADC_PGA_GAIN_8   = ADC_PGA_PGAG(3),
    ADC_PGA_GAIN_16  = ADC_PGA_PGAG(4),
    ADC_PGA_GAIN_32  = ADC_PGA_PGAG(5),
    ADC_PGA_GAIN_64  = ADC_PGA_PGAG(6)
} ADC_PGA_GAIN;
#endif /* ADC_HAS_PGA */
/*
** PDB INSTALL_STRUCT
**
** This structure defines the initialization parameters to be used
** for PDB initialization within ADC init
*/
typedef struct kpdb_install_struct
{
    /* PDB interrupt vector */
    uint32_t IDELAY_VECTOR;

    /* PDB interrupt priority */
    uint32_t IDELAY_PRIORITY;

    /* PDB race condition interrupt vector */
    uint32_t ERROR_VECTOR;

    /* PDB race condition interrupt priority */
    uint32_t ERROR_PRIORITY;

} KPDB_INIT_STRUCT, * KPDB_INIT_STRUCT_PTR;

/*
** ADC_INSTALL_STRUCT
**
** This structure defines the initialization parameters to be used
** when ADC driver is  port is installed.
*/
typedef struct kadc_install_struct
{
    /* The number of ADC peripheral, use adc_t enum from PSP */
    uint8_t ADC_NUMBER;

    /* The clock source */
    ADC_CLOCK_SOURCE CLOCK_SOURCE;

    /* The clock divisor for ADC */
    ADC_CLOCK_DIV CLOCK_DIV;

    /* ADC high speed control, see ADC_HSC enum */
    ADC_HSC SPEED;

    /* ADC low power control, see ADC_LPC enum */
    ADC_LPC POWER;

    /* The calibration data pointer */
    uint8_t *CALIBRATION_DATA_PTR;

    /* ADC interrupt vector */
    uint32_t ADC_VECTOR;

    /* ADC interrupt vector */
    uint32_t ADC_PRIORITY;

    /* KPDB init structure */
    const KPDB_INIT_STRUCT * PDB_INIT;

} KADC_INIT_STRUCT, * KADC_INIT_STRUCT_PTR;

typedef void (_CODE_PTR_ PDB_INT_FCN)(void);

void    *_bsp_get_adc_base_address(_mqx_uint);
void    *_bsp_get_pdb_base_address(void);
int16_t   _pdb_get_prescaler(void);
_mqx_int _pdb_set_prescaler(int16_t prescaler, int16_t multiplier);

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
