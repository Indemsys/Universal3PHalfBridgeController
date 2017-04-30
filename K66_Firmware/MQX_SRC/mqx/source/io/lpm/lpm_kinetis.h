/*HEADER**********************************************************************
*
* Copyright 2011 Freescale Semiconductor, Inc.
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
*   Low Power Manager declarations specific to Kinetis.
*
*
*END************************************************************************/


#ifndef __lpm_kinetis_h__
#define __lpm_kinetis_h__

/*-------------------------------------------------------------------------*/
/*
**                            CONSTANT DEFINITIONS
*/

typedef enum 
{
    LPM_CPU_POWER_MODE_RUN = 0,
    LPM_CPU_POWER_MODE_WAIT,
    LPM_CPU_POWER_MODE_STOP,
    LPM_CPU_POWER_MODE_VLPR,
    LPM_CPU_POWER_MODE_VLPW,
    LPM_CPU_POWER_MODE_VLPS,
    LPM_CPU_POWER_MODE_LLS,
    LPM_CPU_POWER_MODE_VLLS3,
    LPM_CPU_POWER_MODE_VLLS2,
    LPM_CPU_POWER_MODE_VLLS1,
#ifdef MQX_ENABLE_HSRUN
    LPM_CPU_POWER_MODE_HSRUN,
#endif
    LPM_CPU_POWER_MODES
} LPM_CPU_POWER_MODE_INDEX;

#define LPM_CPU_POWER_MODE_FLAG_DEEP_SLEEP          (0x01)
#define LPM_CPU_POWER_MODE_FLAG_USE_WFI             (0x02)
#define LPM_CPU_POWER_MODE_FLAG_SLEEP_ON_EXIT       (0x04)
#define LPM_CPU_POWER_MODE_FLAG_USER_MASK           (LPM_CPU_POWER_MODE_FLAG_SLEEP_ON_EXIT)

#ifndef SMC_PMCTRL_LPWUI_MASK
#define SMC_PMCTRL_LPWUI_MASK                       (0x80)
#endif

/*-------------------------------------------------------------------------*/
/*
**                            MACRO DECLARATIONS
*/


/*-------------------------------------------------------------------------*/
/*
**                            DATATYPE DECLARATIONS
*/

typedef struct lpm_cpu_power_mode {
    /* Mode control register setup */
    uint8_t      PMCTRL;

    /* Flags specifying low power mode behavior */
    uint8_t      FLAGS;

} LPM_CPU_POWER_MODE, * LPM_CPU_POWER_MODE_PTR;


typedef struct lpm_cpu_operation_mode {
    /* Index into predefined cpu operation modes */
    LPM_CPU_POWER_MODE_INDEX    MODE_INDEX;

    /* Additional modification flags */
    uint8_t                      FLAGS;

    /* LLWU specific settings */
    uint8_t                      PE1;
    uint8_t                      PE2;
    uint8_t                      PE3;
    uint8_t                      PE4;
    uint8_t                      ME;
    
} LPM_CPU_OPERATION_MODE, * LPM_CPU_OPERATION_MODE_PTR;


/*-------------------------------------------------------------------------*/
/*
**                            FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

    
extern _mqx_uint _lpm_set_cpu_operation_mode (const LPM_CPU_OPERATION_MODE *, LPM_OPERATION_MODE);
extern void      _lpm_wakeup_core (void);
extern bool      _lpm_idle_sleep_check (void);
extern uint32_t  _lpm_get_reset_source(void);
extern _mqx_uint _lpm_write_rfvbat(uint8_t, uint32_t);
extern _mqx_uint _lpm_read_rfvbat(uint8_t, uint32_t *);
extern _mqx_uint _lpm_write_rfsys(uint8_t, uint32_t);
extern _mqx_uint _lpm_read_rfsys(uint8_t, uint32_t *);


#ifdef __cplusplus
}
#endif


#endif

/* EOF */
