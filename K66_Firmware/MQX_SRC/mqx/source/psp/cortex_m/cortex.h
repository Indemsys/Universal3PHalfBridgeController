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
*   This file contains the type definitions for the ARM Cortex processors.
*
*
*END************************************************************************/

#ifndef __cortex_h__
#define __cortex_h__

/*==========================================================================*/
/*
**                   MQX REQUIRED DEFINITIONS
**
** Other MQX kernel and component functions require these definitions to exist.
*/

/* Indicate which endian this PSP is */
#if defined(__ARM__) || defined(__GHS__)
    #ifdef __BIG_ENDIAN
        #define PSP_ENDIAN          MQX_BIG_ENDIAN
    #else
        #define PSP_ENDIAN          MQX_LITTLE_ENDIAN
    #endif
#elif defined(__MET__)
    #ifdef _BE
        #define PSP_ENDIAN          MQX_BIG_ENDIAN
    #else
        #define PSP_ENDIAN          MQX_LITTLE_ENDIAN
    #endif
#elif defined(__GNUC__)
    #ifdef BYTES_BIG_ENDIAN
        #define PSP_ENDIAN          MQX_BIG_ENDIAN
    #else
        #define PSP_ENDIAN          MQX_LITTLE_ENDIAN
    #endif
#else
    #define PSP_ENDIAN              MQX_LITTLE_ENDIAN
#endif

/*
** Memory alignment requirements.
** The alignment indicates how memory is to be aligned for all memory
** accesses.   This is to avoid mis-aligned transfers of data, thus
** optimizing memory accesses.
*/
#define PSP_MEMORY_ALIGNMENT        (PSP_CACHE_LINE_SIZE-1)
#define PSP_MEMORY_ALIGNMENT_MASK   (~PSP_MEMORY_ALIGNMENT)

/*
** Stack alignment requirements.
** The alignment indicates how the stack is to be initially aligned.
** This is to avoid mis-aligned types on the stack
*/
#define PSP_STACK_ALIGNMENT         PSP_MEMORY_ALIGNMENT
#define PSP_STACK_ALIGNMENT_MASK    PSP_MEMORY_ALIGNMENT_MASK

/*
** Indicate the direction of the stack
*/
#define PSP_STACK_GROWS_TO_LOWER_MEM                (1)

/*
** Indicate addressing capability of the CPU
** This is the memory width. i.e., the number of bits addressed
** by each memory location.
*/
#define PSP_MEMORY_ADDRESSING_CAPABILITY            (8)

/*
** Indicate alignment restrictions on memory accesses
** For an n-bit memory access,
**
** if n <  PSP_MEMORY_ACCESSING_CAPABILITY,
**         n-bit accesses must be n-bit-aligned
**
** if n >= PSP_MEMORY_ACCESSING_CAPABILITY,
**         n-bit accesses must be PSP_MEMORY_ACCESSING_CAPABILITY-bit-aligned
*/
#define PSP_MEMORY_ACCESSING_CAPABILITY             (32)

/*
** Define padding need to make the STOREBLOCK_STRUCT aligned properly
*/
#define PSP_MEM_STOREBLOCK_ALIGNMENT                (2)

/*
**   MINIMUM STACK SIZE FOR A TASK
*/

/*
** Memory overhead on users stack before switching over to the interrupt stack.

*/
#if PSP_MQX_CPU_IS_ARM_CORTEX_M4
    #define PSP_STACK_INTERRUPT_OVERHEAD            (116)
#else /* PSP_MQX_CPU_IS_ARM_CORTEX_M0 is assumed */
    #define PSP_STACK_INTERRUPT_OVERHEAD            (64)
#endif

/* This much extra stack is required for the logging of mqx functions */
#if MQX_KERNEL_LOGGING
    #define PSP_STACK_KLOG_OVERHEAD                 (128)
#else
    #define PSP_STACK_KLOG_OVERHEAD                 (0)
#endif

/* Minimum stack size for all tasks. */
#define PSP_MINSTACKSIZE                      \
            (sizeof(PSP_STACK_START_STRUCT) + \
            PSP_STACK_INTERRUPT_OVERHEAD    + \
            PSP_STACK_KLOG_OVERHEAD)

/* Minimum stack size for the Idle Task
**   Idle task written in C uses at most 8 bytes of stack:
**     1 Link Instruction, 1 local variable (pointer)
*/
#define PSP_IDLE_STACK_SIZE                         (PSP_MINSTACKSIZE + 8)

/* Init task stack size
*/
#define PSP_INIT_STACK_SIZE                         (PSP_MINSTACKSIZE * 10)

/* Motorola addresses do not need to be normalized
** (ie as for example the Intel chips in real mode do require this)
*/
#define _PSP_NORMALIZE_MEMORY(x)                    (x)

/*==========================================================================*/
/*
**                    PSP SPECIFIC DEFINITIONS
**
*/

/*
** The maximum number of hardware interrupt vectors
*/
#if PSP_MQX_CPU_IS_ARM_CORTEX_M4
    #ifndef PSP_MAXIMUM_INTERRUPT_VECTORS
        #define PSP_MAXIMUM_INTERRUPT_VECTORS           (0x100)
    #endif
#elif PSP_MQX_CPU_IS_ARM_CORTEX_M0P
    #ifndef PSP_MAXIMUM_INTERRUPT_VECTORS
        #define PSP_MAXIMUM_INTERRUPT_VECTORS           (0x2F)
    #endif
#else
    #error Unknown ARM Cortex device
#endif

/*
**           The barrier protection
**
** When PSP_MEM_BARRIER_PROTECTION is defined as 1,
** then mqx will check the start of kernel data memory
** against memory barrier at address 0x20000000 and
** reserve a block of memory containing that address.
** Applicable if LWMEM and MEM allocators are used 
** and for Cortex M0P and M4 only.
*/
#if (PSP_MQX_CPU_IS_ARM_CORTEX_M4 || PSP_MQX_CPU_IS_ARM_CORTEX_M0P)
    #ifndef PSP_MEM_BARRIER_PROTECTION
        #define PSP_MEM_BARRIER_PROTECTION               (1)
    #endif
#endif

#ifndef __ASM__

/*
** PSP FUNCTION CALL STRUCT
**
** This structure defines what a function call pushes onto the stack
*/
typedef struct psp_function_call_struct
{

   /* Frame pointer (r11) pushed onto the stack for back tracing */
   void                *FP_REGISTER;

   /* Pushed onto the stack by the JSR instruction */
   void     (_CODE_PTR_ RETURN_ADDRESS)(void);

} PSP_FUNCTION_CALL_STRUCT, * PSP_FUNCTION_CALL_STRUCT_PTR;


/*==========================================================================*/
/*                         MQX DATA TYPES                                   */


/*-----------------------------------------------------------------------*/
/*
** PSP BLOCKED STACK STRUCT
**
** This is what a stack looks like for a task that is NOT the active
** task
*/
typedef struct psp_blocked_stack_struct
{
#if PSP_MQX_CPU_IS_ARM_CORTEX_M4
    uint32_t PENDSVPRIOR;
    uint32_t BASEPRI;
#elif PSP_MQX_CPU_IS_ARM_CORTEX_M0P
    uint32_t PRIMASK;
#endif
    uint32_t R4;
    uint32_t R5;
    uint32_t R6;
    uint32_t R7;
    uint32_t R8;
    uint32_t R9;
    uint32_t R10;
    uint32_t R11;
#if PSP_MQX_CPU_IS_ARM_CORTEX_M4
    uint32_t LR2;
#endif
    uint32_t R0;
    uint32_t R1;
    uint32_t R2;
    uint32_t R3;

    uint32_t R12;
    uint32_t LR;
    uint32_t PC;
    uint32_t PSR;
} PSP_BLOCKED_STACK_STRUCT, * PSP_BLOCKED_STACK_STRUCT_PTR;

#if PSP_HAS_FPU
/*-----------------------------------------------------------------------*/
/*
** PSP BLOCKED FP STRUCT
**
** This structure defines the registers stored by MQX when a floating
** point task is blocked.  When the FLOATING_POINT_REGISTERS_STORED bit
** is set task's FLAGS, then this structure is valid.
*/
typedef struct psp_blocked_fp_struct {
    uint32_t  FPCCR;     /* FP Context Control Register                  */
    uint32_t  FPCAR;     /* FP Context Address Register                  */
    uint32_t  FPDSCR;    /* FP Default Status Control Register           */
    uint32_t  FPSCR;     /* FP Status Control Register                   */
    uint32_t  TID;       /* Testing for correctness                      */
    float S[32];        /* The floating point computation registers     */
} PSP_BLOCKED_FP_STRUCT, * PSP_BLOCKED_FP_STRUCT_PTR;

#endif

/*-----------------------------------------------------------------------*/
/*
** PSP INTERRUPT CONTEXT STRUCT
**
** This structure provides a "context" for mqx primitives to use while executing
** in an interrupt handler.  A link list of these contexts is built on the
** interrupt stack.  The head of this list (the current interrupt context) is
** pointed to by the INTERRUPT_CONTEXT_PTR field of the kernel data structure.
**
*/
typedef struct psp_int_context_struct
{
    /* Address of previous context, NULL if none */
    struct psp_int_context_struct      *PREV_CONTEXT;

    /* The exception number for this interrupt frame */
    uint32_t                             EXCEPTION_NUMBER;

    /* Used by the _int_enable function while in the ISR */
    uint32_t                             ENABLE_SR;

    /* The "task" error code for use by mqx functions while in the ISR */
    uint32_t                             ERROR_CODE;
    
#if !MQX_USE_IO_OLD
    int ERRNO;
#endif // MQX_USE_IO_OLD
    
} PSP_INT_CONTEXT_STRUCT, * PSP_INT_CONTEXT_STRUCT_PTR;


/*-----------------------------------------------------------------------*/
/*
** PSP BASIC INT FRAME STRUCT
**
** This structure is what is pushed onto the memory on the current stack
** when an interrupt occurs.
**
** For the first interrupt that interrupts a running task, this structure
** defines what the top of the stack for the task looks like,
** and the STACK_PTR field of the task descriptor will point to this structure.
**
** The rest of the interrupt frame is then built upon the interrupt stack.
**
** For a nested interrupt, this basic frame is also built upon the interrupt
** stack.
**
*/

typedef struct psp_basic_int_frame_struct {
    uint32_t R0;
    uint32_t R1;
    uint32_t R2;
    uint32_t R3;

    uint32_t R12;
    uint32_t LR;
    uint32_t PC;
    uint32_t PSR;
} PSP_BASIC_INT_FRAME_STRUCT, * PSP_BASIC_INT_FRAME_STRUCT_PTR;

/*-----------------------------------------------------------------------*/
/*
** PSP STACK START STRUCT
**
** This structure is used during the initialization of a new task.
** It is overlaid on the bottom of the task's stack
**
*/
typedef struct psp_stack_start_struct
{
    /* The start up registers for the task */
    PSP_BLOCKED_STACK_STRUCT    INITIAL_CONTEXT;

    /* The end of INITIAL CONTEXT has to be aligned at 8B boundary. Presuming that the end of this structure is
    ** aligned at PSP_STACK_ALIGN boundary, we must add reserved bytes to achieve alignment.
    ** The reserved space is computed from the used space from the end (5 * 32-bit value).
    */
    uint8_t                      RESERVED[(PSP_STACK_ALIGNMENT_MASK - 5 * sizeof(uint32_t)) & 0x7];

    /* Previous stack pointer for exit return */
    void                       *PREVIOUS_STACK_POINTER;

    /* The function to call when the task "returns" */
    void                        (_CODE_PTR_ EXIT_ADDRESS)();

    /* The task's parameter */
    uint32_t                     PARAMETER;

    /* The following two fields are used to create a "bottom" of stack
    ** that debuggers will recognize
    */
    /* End stack pointer    */
    void                       *ZERO_STACK_POINTER;

    /* close the stack frame with a return address of 0 */
    uint32_t                     ZERO_RETURN_ADDRESS;

} PSP_STACK_START_STRUCT, * PSP_STACK_START_STRUCT_PTR;

#define PSP_STACK_PARAMETER PARAMETER


/*--------------------------------------------------------------------------*/
/*
**                  PROTOTYPES OF PSP FUNCTIONS
*/


/*
** PROCESSOR MEMORY BOUNDS
*/
#define CORTEX_PERIPH_BASE                  (0x40000000ul)    /* peripheral base address */
#define CORTEX_PRI_PERIPH_IN_BASE           (0xe0000000ul)    /* private peripheral internal base address */

/* minimal implemented priority required by Cortex core */
#ifndef CORTEX_PRIOR_IMPL
    #if PSP_MQX_CPU_IS_ARM_CORTEX_M0P
        #define CORTEX_PRIOR_IMPL           (1)
    #elif PSP_MQX_CPU_IS_ARM_CORTEX_M4
        #define CORTEX_PRIOR_IMPL           (3)
    #endif
#endif /* CORTEX_PRIOR_IMPL */

#define CORTEX_PRIOR_SHIFT                  (8 - CORTEX_PRIOR_IMPL)
#define CORTEX_PRIOR_MASK                   ((0xff << CORTEX_PRIOR_SHIFT) & 0xff)
#define CORTEX_PRIOR(x)                     (((x) << CORTEX_PRIOR_SHIFT) & CORTEX_PRIOR_MASK)

#define CORTEX_INT_FIRST_INTERNAL           (0)

#ifndef CORTEX_INT_LAST_INTERNAL
#if PSP_MQX_CPU_IS_ARM_CORTEX_M0P
    #define CORTEX_INT_LAST_INTERNAL        (32)
#elif PSP_MQX_CPU_IS_ARM_CORTEX_M4
    #define CORTEX_INT_LAST_INTERNAL        (250)
#endif
#endif /* CORTEX_INT_LAST_INTERNAL */

#define PSP_INT_FIRST_INT_ROUTER            (16)

#define PSP_INT_FIRST_INTERNAL              (CORTEX_INT_FIRST_INTERNAL)
#define PSP_INT_LAST_INTERNAL               (CORTEX_INT_LAST_INTERNAL)

#define CORTEX_MPU_REC                      (12)

#define MPU_SM_RWX                          (0)
#define MPU_SM_RX                           (1)
#define MPU_SM_RW                           (2)
#define MPU_SM_AS_USER                      (3)

#define MPU_UM_R                            (4)
#define MPU_UM_W                            (2)
#define MPU_UM_X                            (1)

#define MPU_UM_RW                           (MPU_UM_R | MPU_UM_W)

/*
** CORTEX_SCS_STRUCT
** Reset and Clock Control
*/
typedef struct cortex_syst_struct {
    uint32_t CSR;      /* SysTick Control and Status */
    uint32_t RVR;      /* SysTick Reload value       */
    uint32_t CVR;      /* SysTick Current value      */
    uint32_t CALIB;    /* SysTick Calibration value  */
} CORTEX_SYST_STRUCT, * CORTEX_SYST_STRUCT_PTR;
typedef volatile struct cortex_syst_struct * VCORTEX_SYST_STRUCT_PTR;

/*
** CORTEX_NVIC_STRUCT
** Reset and Clock Control
*/
typedef struct cortex_nvic_struct {
    uint32_t ENABLE[32];
    uint32_t DISABLE[32];
    uint32_t SET[32];
    uint32_t CLR[32];
    uint32_t ACTIVE[32];
    uint32_t rsvd[32];
    uint32_t PRIORITY[32];
} CORTEX_NVIC_STRUCT, * CORTEX_NVIC_STRUCT_PTR;
typedef volatile struct cortex_nvic_struct * VCORTEX_NVIC_STRUCT_PTR;

/*
** CORTEX_SCB_STRUCT
** system control block
*/
typedef struct cortex_scb_struct {
    uint32_t CPUID;
    uint32_t ICSR;
    uint32_t VTOR;
    uint32_t AIRCR;
    uint32_t SCR;
    uint32_t CCR;
    uint32_t SHPR1;
    uint32_t SHPR2;
    uint32_t SHPR3;
    uint32_t SHCRS;
    uint32_t CFSR;
    uint32_t HFSR;
} CORTEX_SCB_STRUCT, * CORTEX_SCB_STRUCT_PTR;
typedef volatile struct cortex_scb_struct * VCORTEX_SCB_STRUCT_PTR;

/*
** CORTEX__SCS_STRUCT
** This structure defines the memory/registers provided by the CORTEX core
** 0xE0000000 - 0xE0100000
*/
typedef struct cortex_scs_struct
{
    unsigned char           filler0[0xe010];
    CORTEX_SYST_STRUCT      SYST;
    unsigned char           filler1[0xE000E100 - (0xE000E010 + sizeof(CORTEX_SYST_STRUCT))];
    CORTEX_NVIC_STRUCT      NVIC;
    unsigned char           filler2[0xE000ED00 - (0xE000E100 + sizeof(CORTEX_NVIC_STRUCT))];
    CORTEX_SCB_STRUCT       SCB;
} CORTEX_SCS_STRUCT, * CORTEX_SCS_STRUCT_PTR;
typedef volatile struct cortex_scs_struct * VCORTEX_SCS_STRUCT_PTR;

#define _PSP_GET_VTOR()     (_mqx_max_type)(((VCORTEX_SCS_STRUCT_PTR)CORTEX_PRI_PERIPH_IN_BASE)->SCB.VTOR);
#define _PSP_SET_VTOR(x)    ((VCORTEX_SCS_STRUCT_PTR)CORTEX_PRI_PERIPH_IN_BASE)->SCB.VTOR = ((uint32_t)(x));



/* Interrupt Acknowledge Level and Priority Register
**
** Interrupt priority can range 0 (lowest) to 7 (highest priority) and
** a special value 8 (mid-point) for fixed level interrupts
*/
typedef uint8_t _int_level;
typedef uint8_t _int_priority;

/*--------------------------------------------------------------------------*/
/*
**                  PROTOTYPES OF PSP FUNCTIONS
*/

#ifdef __cplusplus
extern "C" {
#endif

/* Generic PSP prototypes */
_mqx_uint _psp_int_init(_mqx_uint, _mqx_uint);
void _psp_int_install(void);
uint32_t _psp_core_num(void);

/* Prototypes of assembler functions */
void _psp_push_fp_context(void);
void _psp_pop_fp_context(void);

#ifndef __IAR_SYSTEMS_ICC__
#ifndef __set_MSP
extern void             __set_MSP(uint32_t);
#endif

#ifndef __get_PSP
extern uint32_t         __get_PSP(void);
#endif

#ifndef __get_MSP
extern uint32_t         __get_MSP(void);
#endif

#ifndef __get_PSR
extern uint32_t         __get_PSR(void);
#endif

#ifndef __get_LR
extern uint32_t         __get_LR(void);
#endif

#ifndef __get_PC
extern uint32_t         __get_PC(void);
#endif

#ifndef __get_CONTROL
extern uint32_t         __get_CONTROL(void);
#endif

#endif /* __IAR_SYSTEMS_ICC__ */

/* Enable and disable ALL interrupts on ARM CortexM processors.
  Some compilers export this function as pre-defined macro                    */
#ifndef __enable_interrupt
extern void         __enable_interrupt(void);
#endif
#ifndef __disable_interrupt
extern void         __disable_interrupt(void);
#endif

#endif /* __ASM__ */

#ifdef __cplusplus
}
#endif

#endif

