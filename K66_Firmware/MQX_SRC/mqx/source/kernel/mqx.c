
/*HEADER**********************************************************************
*
* Copyright 2008-2010 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
* Copyright 1989-2008 ARC International
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains the source for the main MQX function _mqx().
*
*
*END************************************************************************/

#ifndef __NO_SETJMP
#include <setjmp.h>
#endif

#include "mqx_inc.h"
#include "gen_rev.h"
#include "psp_rev.h"

/*
 * A global zero initialized MQX tick structure.
 * It is used by various MQX functions that need
 * to zero initialize local tick structures. An
 * extern to it is provided in MQX.H so applications
 * can use it as well.
 */
const MQX_TICK_STRUCT _mqx_zero_tick_struct = {0};

/* version/vendor identification also used by TAD to determine paths to its context files */
const uint32_t          _mqx_version_number = 0x04020000;
const char      *const _mqx_vendor         = "Freescale/Freescale MQX";

/* Identify the product */
#if MQX_CRIPPLED_EVALUATION
const char      *const _mqx_version        = "4.2.0c";
#else
const char      *const _mqx_version        = "4.2.0";
#endif

/* version strings */
const char      *const _mqx_generic_revision = REAL_NUM_TO_STR(GEN_REVISION);
const char      *const _mqx_psp_revision     = REAL_NUM_TO_STR(PSP_REVISION);
const char      *const _mqx_copyright = "(c) 2013 Freescale Semiconductor. All rights reserved.";
const char      *const _mqx_date      = __DATE__ " at " __TIME__;

/* A global pointer to the location of the kernel data structure */
KERNEL_ACCESS struct kernel_data_struct      *_mqx_kernel_data = (void *)-1;
KERNEL_ACCESS volatile void   *_mqx_system_stack;

#if PSP_BYPASS_P3_WFI
void (*_sleep_p3_ram)(uint32_t*);
uint32_t _sleep_p3_loop_ram;
uint32_t _sleep_p3_wfi_ram;
#endif

/* Error return jump buffer for kernel errors */
#if MQX_EXIT_ENABLED || MQX_CRIPPLED_EVALUATION
jmp_buf _mqx_exit_jump_buffer_internal;
#endif

/* TODO copied from monitor.c find suitable place */
volatile _WEAK_SYMBOL(_mqx_uint _mqx_monitor_type) = MQX_MONITOR_TYPE_NONE;

/*!
 * \brief Initializes and starts MQX on the processor.
 *
 * The function does the following:
 * \li Initializes the default memory pool and memory components.
 * \li Initializes kernel data.
 * \li Performs BSP-specific initialization, which includes installing the
 * periodic timer.
 * \li Performs PSP-specific initialization.
 * \li Creates the interrupt stack.
 * \li Creates the ready queues.
 * \li Starts MQX tasks.
 * \li Starts autostart application tasks.
 *
 * \param[in] mqx_init Pointer to the MQX initialization structure for the
 * processor.
 *
 * \return Does not return (Success.)
 * \return If application called _mqx_exit(), error code that it passed to
 * _mqx_exit() (Success.)
 * \return Errors from _int_install_isr() (MQX cannot install the interrupt
 *  subsystem.)
 * \return Errors from _io_init() (MQX cannot install the I/O subsystem.)
 * \return Errors from _mem_alloc_system() (There is not enough memory to
 * allocate either the interrupt stack or the interrupt table.)
 * \return Errors from _mem_alloc_zero() (There is not enough memory to allocate
 * the ready queues.)
 * \return MQX_KERNEL_MEMORY_TOO_SMALL (Init_struct_ptr does not specify enough
 * kernel memory.)
 * \return MQX_OUT_OF_MEMORY (There is not enough memory to allocate either the
 * ready queues, the interrupt stack, or the interrupt table.)
 * \return MQX_TIMER_ISR_INSTALL_FAIL (MQX cannot install the periodic timer ISR.)
 *
 * \warning Must be called exactly once per processor.
 *
 * \see _mqx_exit
 * \see _int_install_isr
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see MQX_INITIALIZATION_STRUCT
 * \see TASK_TEMPLATE_STRUCT
 */
_mqx_uint _mqx
(
    register MQX_INITIALIZATION_STRUCT_PTR mqx_init
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    TD_STRUCT_PTR td_ptr;
    _mqx_uint result;
    _mqx_uint stack_size;
    void   *stack_ptr;
    void   *sys_td_stack_ptr;
    unsigned char *sys_stack_base_ptr;

#if MQX_EXIT_ENABLED || MQX_CRIPPLED_EVALUATION
    /* Setup a longjmp buffer using setjmp, so that if an error occurs
     * in mqx initialization, we can perform a longjmp to this location.
     *
     * Also _mqx_exit will use this jumpbuffer to longjmp to here in order
     * to cleanly exit MQX.
     */
    if ( MQX_SETJMP( _mqx_exit_jump_buffer_internal ) ) {
        _GET_KERNEL_DATA(kernel_data);
        _int_set_vector_table(kernel_data->USERS_VBR);
        return kernel_data->USERS_ERROR;
    } /* Endif */
#endif

    /*
     * The kernel data structure starts at the start of kernel memory,
     * as specified in the initialization structure. Make sure address
     * specified is aligned
     */
    kernel_data = (KERNEL_DATA_STRUCT_PTR) _ALIGN_ADDR_TO_HIGHER_MEM(mqx_init->START_OF_KERNEL_MEMORY);

    /* Set the global pointer to the kernel data structure */
    _SET_KERNEL_DATA(kernel_data);

    /* The following assignments are done to force the linker to include
     * the symbols, which are required by TAD.
     * Note that we should use address of the variable so it is not optimized
     * as direct constant assignment when optimization level is high.
     * Note that counter will be immediately reset to zero on the subsequent
     * _mem_zero call. */
    *(volatile  void **) kernel_data = (void *) & _mqx_version_number;
    *(volatile  void **) kernel_data = (void *) & _mqx_vendor;

    /* Initialize the kernel data to zero. */
    _mem_zero((void *) kernel_data, (_mem_size) sizeof(KERNEL_DATA_STRUCT));

#if MQX_CHECK_ERRORS && MQX_VERIFY_KERNEL_DATA
    /* Verify that kernel data can be read and written correcly without
     * errors.  This is necessary during BSP development to validate the
     * DRAM controller is initialized properly.
     */

#ifndef PSP_KERNEL_DATA_VERIFY_ENABLE
#define PSP_KERNEL_DATA_VERIFY_ENABLE   0
#endif /* PSP_KERNEL_DATA_VERIFY_ENABLE */
    if (PSP_KERNEL_DATA_VERIFY_ENABLE) {
        /* This memory check is dangerous, because can destroy boot stack
         * stack which is used !!! -> MQX will failed !
         * Set PSP_KERNEL_DATA_VERIFY_ENABLE to 1
         * only if your boot stack is out of MQX memory heap
         */

        result = _mem_verify((unsigned char *)kernel_data + sizeof(KERNEL_DATA_STRUCT),
                        mqx_init->END_OF_KERNEL_MEMORY);
        if ( result != MQX_OK ) {
            _mqx_exit(result); /* RETURN TO USER */
        }
    }
#endif /* MQX_CHECK_ERRORS && MQX_VERIFY_KERNEL_DATA */
    /* Copy the MQX initialization structure into kernel data. */
    kernel_data->INIT = *mqx_init;
    kernel_data->INIT.START_OF_KERNEL_MEMORY = (void *) kernel_data;
    kernel_data->INIT.END_OF_KERNEL_MEMORY = (void *) _ALIGN_ADDR_TO_LOWER_MEM(kernel_data->INIT.END_OF_KERNEL_MEMORY);

    /* init kernel data structures */
    _mqx_init_kernel_data_internal();

    /* Initialize the memory resource manager for the kernel */
    result = _mem_init_internal();
#if MQX_CHECK_ERRORS
    if ( result != MQX_OK ) {
        _mqx_exit(result); /* RETURN TO USER */
    } /* Endif */
#endif

#if MQX_USE_INTERRUPTS

    /* Now obtain the interrupt stack */
    if (kernel_data->INIT.INTERRUPT_STACK_LOCATION) {
        stack_ptr = kernel_data->INIT.INTERRUPT_STACK_LOCATION;
        stack_size = kernel_data->INIT.INTERRUPT_STACK_SIZE;
    }
    else {
        if ( kernel_data->INIT.INTERRUPT_STACK_SIZE < PSP_MINSTACKSIZE ) {
            kernel_data->INIT.INTERRUPT_STACK_SIZE = PSP_MINSTACKSIZE;
        } /* Endif */
#if PSP_STACK_ALIGNMENT
        stack_size = kernel_data->INIT.INTERRUPT_STACK_SIZE + PSP_STACK_ALIGNMENT + 1;
#else
        stack_size = kernel_data->INIT.INTERRUPT_STACK_SIZE;
#endif
        stack_ptr = _mem_alloc_system((_mem_size)stack_size);
#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
        if (stack_ptr == NULL) {
            _mqx_exit(MQX_OUT_OF_MEMORY); /* RETURN TO USER */
        } /* Endif */
#endif
        _mem_set_type(stack_ptr, MEM_TYPE_INTERRUPT_STACK);
    } /* Endif */

#if MQX_MONITOR_STACK
    _task_fill_stack_internal((_mqx_uint_ptr)stack_ptr, stack_size);
#endif

    kernel_data->INTERRUPT_STACK_PTR = _GET_STACK_BASE(stack_ptr, stack_size);

#if PSP_MQX_CPU_IS_VYBRID_A5
    /* FIQ stack */
    stack_ptr = _mem_alloc_system((_mem_size)stack_size);
#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
    if (stack_ptr == NULL) {
        _mqx_exit(MQX_OUT_OF_MEMORY);
    }
#endif
    _mem_set_type(stack_ptr, MEM_TYPE_INTERRUPT_STACK);
#if MQX_MONITOR_STACK
    _task_fill_stack_internal((_mqx_uint_ptr)stack_ptr, stack_size);
#endif
    extern PSP_BASIC_INT_FRAME_STRUCT_PTR _fiq_sp;
    _fiq_sp = _GET_STACK_BASE(stack_ptr, stack_size);

    /* ABORT stack */
    stack_ptr = _mem_alloc_system((_mem_size)stack_size);
#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
    if (stack_ptr == NULL) {
        _mqx_exit(MQX_OUT_OF_MEMORY);
    }
#endif
    _mem_set_type(stack_ptr, MEM_TYPE_INTERRUPT_STACK);
#if MQX_MONITOR_STACK
    _task_fill_stack_internal((_mqx_uint_ptr)stack_ptr, stack_size);
#endif
    extern PSP_BASIC_INT_FRAME_STRUCT_PTR _abort_sp;
    _abort_sp = _GET_STACK_BASE(stack_ptr, stack_size);


    /* UNDEF stack */
    stack_ptr = _mem_alloc_system((_mem_size)stack_size);
#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
    if (stack_ptr == NULL) {
        _mqx_exit(MQX_OUT_OF_MEMORY);
    }
#endif
    _mem_set_type(stack_ptr, MEM_TYPE_INTERRUPT_STACK);
#if MQX_MONITOR_STACK
    _task_fill_stack_internal((_mqx_uint_ptr)stack_ptr, stack_size);
#endif
    extern PSP_BASIC_INT_FRAME_STRUCT_PTR _undef_sp;
    _undef_sp = _GET_STACK_BASE(stack_ptr, stack_size);
#endif // PSP_MQX_CPU_IS_VYBRID_A5

#endif

    /*
     * Set the stack for the system TD, in case the idle task gets blocked
     * by an exception or if idle task is not used.
     */
    result = PSP_MINSTACKSIZE;
    sys_td_stack_ptr = _mem_alloc_system((_mem_size) result);
#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
    if (sys_td_stack_ptr == NULL) {
        _mqx_exit(MQX_OUT_OF_MEMORY); /* RETURN TO USER */
    } /* Endif */
#endif
    _mem_set_type(sys_td_stack_ptr, MEM_TYPE_SYSTEM_STACK);

    sys_stack_base_ptr = (unsigned char *) _GET_STACK_BASE(sys_td_stack_ptr, result);
    td_ptr = SYSTEM_TD_PTR(kernel_data);
    td_ptr->STACK_PTR = (void *)(sys_stack_base_ptr - sizeof(PSP_STACK_START_STRUCT));
    td_ptr->STACK_BASE = sys_stack_base_ptr;
#if MQX_TD_HAS_STACK_LIMIT
    td_ptr->STACK_LIMIT = _GET_STACK_LIMIT(sys_td_stack_ptr, result);
#endif
    _mqx_system_stack = td_ptr->STACK_PTR;

    /* Build the MQX ready to run queues */
    result = _psp_init_readyqs();
#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
    if ( result != MQX_OK ) {
        _mqx_exit(result); /* RETURN TO USER */
    } /* Endif */
#endif

#if MQX_USE_COMPONENTS

    /* Create a light wait semaphore for component creation */
    _lwsem_create((LWSEM_STRUCT_PTR)&kernel_data->COMPONENT_CREATE_LWSEM, 1);
#endif

    /* Create a light wait semaphore for task creation/destruction creation */
    _lwsem_create((LWSEM_STRUCT_PTR) & kernel_data->TASK_CREATE_LWSEM, 1);

    result = _bsp_pre_init();        // only basic init - timer, int
    if ( result != MQX_OK ) {
        _mqx_exit(result);
    }

    /* Create the init task */
    td_ptr = _task_init_internal((TASK_TEMPLATE_STRUCT_PTR)&kernel_data->INIT_TASK_TEMPLATE, kernel_data->ACTIVE_PTR->TASK_ID, MQX_INIT_TASK_PARAMETER, TRUE, NULL, 0);
    if (td_ptr == NULL) {
        _mqx_exit(MQX_OUT_OF_MEMORY);
    }
    _task_ready_internal(td_ptr);

#if MQX_HAS_TIME_SLICE
    /* Set the kernel default time slice value */
    PSP_ADD_TICKS_TO_TICK_STRUCT(&kernel_data->SCHED_TIME_SLICE,
                    MQX_DEFAULT_TIME_SLICE, &kernel_data->SCHED_TIME_SLICE);
#endif

    /* Create the idle task */
#if MQX_USE_IDLE_TASK

#if PSP_BYPASS_P3_WFI
    {
        extern uint32_t _sleep_p3_start[];
        extern uint32_t _sleep_p3_end[];
        extern uint32_t _sleep_p3_loop[];
        extern uint32_t _sleep_p3_wfi[];
        void *p;

        /* allocate uncached memory for task code */
        p = _mem_alloc_uncached(((uint32_t)_sleep_p3_end & 0xfffffffe) - ((uint32_t)_sleep_p3_start & 0xfffffffe));
        _sleep_p3_ram = (void(*)(uint32_t*))( ( ((uint32_t)_sleep_p3 & 0xfffffffe)- ((uint32_t)_sleep_p3_start  & 0xfffffffe) + ((uint32_t)p  & 0xfffffffe) ) | 1);
        _sleep_p3_loop_ram = ((uint32_t)_sleep_p3_loop & 0xfffffffe) - ((uint32_t)_sleep_p3_start & 0xfffffffe) + (uint32_t)p;
        _sleep_p3_wfi_ram = ((uint32_t)_sleep_p3_wfi & 0xfffffffe) - ((uint32_t)_sleep_p3_start & 0xfffffffe) + (uint32_t)p;

        /* copy idle task to mem */
        _mem_copy((void*)((uint32_t)_sleep_p3_start & 0xfffffffe), p, ((uint32_t)_sleep_p3_end & 0xfffffffe) - ((uint32_t)_sleep_p3_start & 0xfffffffe));
    }
#endif

    td_ptr = _task_init_internal((TASK_TEMPLATE_STRUCT_PTR)&kernel_data->IDLE_TASK_TEMPLATE,
                                 kernel_data->ACTIVE_PTR->TASK_ID, MQX_IDLE_TASK_PARAMETER, TRUE, NULL, 0);

    if (td_ptr == NULL) {
        _mqx_exit(MQX_OUT_OF_MEMORY);
    }
    _task_ready_internal(td_ptr);
#endif

    _sched_start_internal(); /* WILL NEVER RETURN FROM HERE */

    return MQX_OK; /* To satisfy lint */

} /* Endbody */

/*!
 * \brief Terminate the MQX application and return to the environment that
 * started the application.
 *
 * The function returns back to the environment that called _mqx(). If the
 * application has installed the MQX exit handler (_mqx_set_exit_handler()),
 * _mqx_exit() calls the MQX exit handler before it exits. By default,
 * _bsp_exit_handler is installed as the MQX exit handler in each BSP.
 *
 * \note It is important to ensure that the environment (boot call stack) the
 * MQX is returning to is in the consistent state. This is not provided by
 * distributed MQX BSPs, because the boot stack is reused (rewritten) by MQX
 * Kernel data. Set the boot stack outside of Kernel data section to support
 * correct _mqx_exit() functionality.
 *
 * \param[in] error Error code to return to the function that called _mqx().
 *
 * \warning Behavior depends on the BSP.
 *
 * \see _mqx
 */
void _mqx_exit
(
    _mqx_uint error
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR  kernel_data = NULL;
    (void)                  kernel_data; /* suppress 'unused variable' warning */

    _int_disable();
    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_mqx_exit, error);

#if MQX_EXIT_ENABLED || MQX_CRIPPLED_EVALUATION
    kernel_data->USERS_ERROR = error;
    if (kernel_data->EXIT_HANDLER) {
        (*kernel_data->EXIT_HANDLER)();
    }/* Endif */
    MQX_LONGJMP( _mqx_exit_jump_buffer_internal, 1 );
#else
    while (TRUE) {
    } /* Endwhile */
#endif
} /* Endbody */

/*!
 * \private
 *
 * \brief Check stack statictics and run context switch handler.
 *
 * \see _mqx_set_context_switch_handler
 */
void _mqx_context_switch_internal
(
    void
)
{ /* Body */

#if MQX_HAS_CONTEXT_SWITCH_HANDLER
    register KERNEL_DATA_STRUCT_PTR  kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    if (kernel_data->CONTEXT_SWITCH_HANDLER)
    {
        TD_STRUCT_PTR old_td_ptr;
        old_td_ptr = kernel_data->OLD_ACTIVE_PTR;

        if (kernel_data->OLD_ACTIVE_PTR != NULL)
        {
            bool stack_error = FALSE;
            /* check stack if the task which has context saved is not a system task */
            if (!(old_td_ptr->TEMPLATE_INDEX & SYSTEM_TASK_FLAG))
            {
#if MQX_TD_HAS_STACK_LIMIT
#if PSP_STACK_GROWS_TO_LOWER_MEM
                stack_error = (char*)old_td_ptr->STACK_PTR < (char*)old_td_ptr->STACK_LIMIT;
#else
                stack_error = (char*)old_td_ptr->STACK_PTR > (char*)old_td_ptr->STACK_LIMIT;
#endif /* PSP_STACK_GROWS_TO_LOWER_MEM */

#if MQX_MONITOR_STACK
                stack_error |= *((_mqx_uint *)old_td_ptr->STACK_LIMIT) != MQX_STACK_MONITOR_VALUE;
#endif /* MQX_MONITOR_STACK */
#endif /* MQX_TD_HAS_STACK_LIMIT */
            } /* Endif */

           (*kernel_data->CONTEXT_SWITCH_HANDLER)(kernel_data->ACTIVE_PTR->TASK_ID, old_td_ptr->TASK_ID, stack_error);

            /* TODO: Should we remove the task which has stack error from its queue */
            if ((stack_error) && (old_td_ptr != kernel_data->ACTIVE_PTR))
            {
                _INT_DISABLE();
                if (old_td_ptr->STATE != UNHANDLED_INT_BLOCKED)
                {
                   old_td_ptr->STATE = UNHANDLED_INT_BLOCKED;
                   _QUEUE_UNLINK(old_td_ptr);
                }
                _INT_ENABLE();
            }
        } /* Endif */
    } /* Endif */

    kernel_data->OLD_ACTIVE_PTR = kernel_data->ACTIVE_PTR;
#endif
} /* Endbody */

/*!
 * \brief Sets the address of the MQX context switch handler, which MQX calls during context switching.
 *
 * \param[in] entry Pointer to the context switch handler.
 *
 * \return Pointer to the previous address
 *
 * \see _mqx_context_switch_internal
 */
void* _mqx_set_context_switch_handler
(
    MQX_CONTEXT_SWITCH_FPTR entry
)
{ /* Body */
#if MQX_HAS_CONTEXT_SWITCH_HANDLER
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void* old_handler;

    _GET_KERNEL_DATA(kernel_data);
    old_handler = (void*)kernel_data->CONTEXT_SWITCH_HANDLER;
    kernel_data->CONTEXT_SWITCH_HANDLER = entry;

    return old_handler;
#else
    return NULL;
#endif
} /* Endbody */

/*!
 * \brief Gets a pointer to kernel data.
 *
 * The address of kernel data corresponds to START_OF_KERNEL_MEMORY in the MQX
 * initialization structure that the application used to start MQX on the
 * processor.
 *
 * \return Pointer to kernel data
 *
 * \see _mqx
 * \see MQX_INITIALIZATION_STRUCT
 */
void *_mqx_get_kernel_data
(
    void
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    return (void *) kernel_data;

} /* Endbody */

/*!
 * \brief Indicates that an error occurred that is so severe that MQX or the
 * application can no longer function.
 *
 * The function logs an error in kernel log (if it has been created and
 * configured to log errors) and calls _mqx_exit().
 * \n MQX calls _mqx_fatal_error() if it detects an unhandled interrupt while it
 * is in _int_exception_isr().
 * \n If an application calls _mqx_fatal_error() when it detects a serious error,
 * you can use this to help you debug by setting a breakpoint in the function.
 *
 * \param[in] error Error code.
 *
 * \warning Terminates the application by calling _mqx_exit().
 *
 * \see _mqx_exit
 * \see _mqx
 * \see _int_exception_isr
 */
void _mqx_fatal_error
(
    _mqx_uint error
)
{ /* Body */
    _KLOGM(KERNEL_DATA_STRUCT_PTR kernel_data);

    _KLOGM(_GET_KERNEL_DATA(kernel_data));
    _KLOGE2(KLOG_mqx_fatal_error, error);
    _mqx_exit(error);
    _KLOGX1( KLOG_mqx_fatal_error);

} /* Endbody */

#if MQX_KD_HAS_COUNTER
/*!
 * \brief Gets a unique number.
 *
 * This function increments the counter and then returns value of the counter.
 * \n This provides a unique number for whoever requires it.
 * \n Note that this unique number will never be 0.
 *
 * \return 16-bit number for 16-bit processors or a 32-bit number for 32-bit
 * processors (unique for the processor and never 0).
 */
_mqx_uint _mqx_get_counter
(
    void
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;
    _mqx_uint return_value;

    _GET_KERNEL_DATA(kernel_data);
    _INT_DISABLE();

    /*
     * Increment counter, and ensure it is not zero.
     * If it is zero, set it to one.
     */
    if ( ++kernel_data->COUNTER == 0 ) {
        kernel_data->COUNTER = 1;
    } /* Endif */
    return_value = kernel_data->COUNTER;
    _INT_ENABLE();
    return (return_value);

} /* Endbody */
#endif /* MQX_KD_HAS_COUNTER */

/*!
 * \brief Gets number of loops of the idle task since restart.
 *
 * \param[out] loop Pointer to idle loop structure.
 *
 * \return MQX_OK
 * \return MQX_IO_OPERATION_NOT_AVAILABLE (Idle loop was not created.)
 */
_mqx_uint _mqx_get_idle_loop_count
(
    IDLE_LOOP_STRUCT_PTR loop
)
{
#if MQX_USE_IDLE_TASK
    register KERNEL_DATA_STRUCT_PTR  kernel_data;
    _GET_KERNEL_DATA(kernel_data);

    *loop = kernel_data->IDLE_LOOP;
    return MQX_OK;
#else /* MQX_USE_IDLE_TASK */
    return MQX_IO_OPERATION_NOT_AVAILABLE;
#endif /* MQX_USE_IDLE_TASK */
}

/*!
 * \brief Gets the type of CPU in the CPU_TYPE field of the kernel data.
 *
 * CPU types begin with PSP_CPU_TYPE_ and are defined in
 * "source\psp\cpu_family\cpu_family.h".
 *
 * \return CPU type.
 *
 * \see _mqx_set_cpu_type
 */
_mqx_uint _mqx_get_cpu_type
(
    void
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);
    return kernel_data->CPU_TYPE;
} /* Endbody */

/*!
 * \brief Sets the type of CPU in the CPU_TYPE field of the kernel data.
 *
 * The function sets CPU_TYPE in kernel data. The MQX Host Tools family of
 * products uses CPU type. CPU types begin with PSP_CPU_TYPE_ and are defined in
 * "source\psp\cpu_family\cpu_family.h".
 *
 * \param[in] cpu_type CPU type to set.
 *
 * \warning Does not verify that cpu_type is valid.
 *
 * \see _mqx_get_cpu_type
 * \see MQX_INITIALIZATION_STRUCT
 */
void _mqx_set_cpu_type
(
    _mqx_uint cpu_type
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);
    kernel_data->CPU_TYPE = cpu_type;

} /* Endbody */

/*!
 * \brief Gets the address of the MQX initialization structure for this processor.
 *
 * \return Pointer to the MQX initialization structure in kernel data.
 *
 * \see _mqx
 * \see MQX_INITIALIZATION_STRUCT
 */
MQX_INITIALIZATION_STRUCT_PTR _mqx_get_initialization
(
    void
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);
    return ((MQX_INITIALIZATION_STRUCT_PTR) & kernel_data->INIT);

} /* Endbody */

#if  MQX_TAD_RESERVED_ENABLE
/*!
 * \brief Gets the TAD_RESERVED field from the task descriptor.
 *
 * Third-party compilers can use the functions in their runtime libraries.
 *
 * \param[in] td Task descriptor to use.
 *
 * \return TAD_RESERVED for Task Descriptor.
 */
void *_mqx_get_tad_data
(
    void   *td
)
{ /* Body */
    TD_STRUCT_PTR td_ptr = (TD_STRUCT_PTR)td;

    return td_ptr->TAD_RESERVED;

} /* Endbody */

/*!
 * \brief Sets the TAD_RESERVED field in the task descriptor.
 *
 * Third-party compilers can use the functions in their runtime libraries.
 *
 * \param[in] td       Task descriptor to use.
 * \param[in] tad_data New value for TAD_RESERVED.
 */
void _mqx_set_tad_data
(
    void   *td,
    void   *tad_data
)
{ /* Body */
    TD_STRUCT_PTR td_ptr = (TD_STRUCT_PTR)td;

    td_ptr->TAD_RESERVED = tad_data;

} /* Endbody */

#endif /* MQX_TAD_RESERVED_ENABLE */

/*!
 * \brief Gets the address of the mqx exit handler function, called when MQX exits.
 *
 * \return Pointer to the MQX exit handler.
 * \return NULL (Exit disabled.)
 *
 * \see _mqx_exit
 * \see _mqx_set_exit_handler
 */
MQX_EXIT_FPTR _mqx_get_exit_handler (void)
{ /* Body */
#if MQX_EXIT_ENABLED
    KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);
    return (kernel_data->EXIT_HANDLER);

#else
    return NULL;
#endif /* MQX_EXIT_ENABLED */
} /* Endbody */

/*!
 * \brief Sets the address of the MQX exit handler, which MQX calls when it exits.
 *
 * \param[in] entry Pointer to the exit handler.
 *
 * \see _mqx_get_exit_handler
 * \see _mqx_exit
 */
void _mqx_set_exit_handler
(
    MQX_EXIT_FPTR entry
)
{ /* Body */
#if MQX_EXIT_ENABLED
    KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_mqx_set_exit_handler, entry);
    kernel_data->EXIT_HANDLER = entry;
    _KLOGX1(KLOG_mqx_set_exit_handler);

#endif /* MQX_EXIT_ENABLED */
} /* Endbody */

#if MQX_USE_IO_COMPONENTS

/*!
 * \brief Gets the I/O Component handle for the specified I/O component.
 *
 * \param[in] component The component number.
 *
 * \return Component handle.
 * \return NULL (Failure.)
 *
 * \warning On failure, calls _task_set_error() to set the following task error
 * code:
 * \li MQX_INVALID_PARAMETER
 *
 * \see _mqx_set_io_component_handle
 */
void *_mqx_get_io_component_handle
(
    _mqx_uint component
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

#if MQX_CHECK_ERRORS
    if (component >= MAX_IO_COMPONENTS) {
        _task_set_error(MQX_INVALID_PARAMETER);
        return(NULL);
    } /* Endif */
#endif

    return kernel_data->IO_COMPONENTS[component];

} /* Endbody */

/*!
 * \brief Sets the I/O Component handle for the specified I/O component and
 * returns the previous value.
 *
 * \param[in] component The component number.
 * \param[in] handle    The new handle.
 *
 * \return Old component handle.
 * \return NULL (Failure.)
 *
 * \warning On failure, calls _task_set_error() to set the following task error
 * code:
 * \li MQX_INVALID_PARAMETER
 *
 * \see _mqx_get_io_component_handle
 */
void *_mqx_set_io_component_handle
(
    _mqx_uint component,
    void     *handle
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;
    void   *old_handle;

    _GET_KERNEL_DATA(kernel_data);

#if MQX_CHECK_ERRORS
    if (component >= MAX_IO_COMPONENTS) {
        _task_set_error(MQX_INVALID_PARAMETER);
        return(NULL);
    } /* Endif */
#endif

    _int_disable();
    old_handle = kernel_data->IO_COMPONENTS[component];
    if (old_handle == NULL) {
        kernel_data->IO_COMPONENTS[component] = handle;
    }
    else {
        if (handle == NULL) {
            kernel_data->IO_COMPONENTS[component] = handle;
        } /* Endif */
    } /* Endif */
    _int_enable();
    return old_handle;

} /* Endbody */

/*!
 * \brief Gets the task ID of the System Task.
 *
 * System resources are owned by System Task.
 *
 * \return Task ID of System Task
 *
 * \see _mem_transfer
 */
_task_id _mqx_get_system_task_id
(
    void
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);
    return(kernel_data->SYSTEM_TD.TASK_ID);

} /* Endbody */

/*!
 * \brief Adds new component handle.
 *
 * \param[in] component      The component number.
 * \param[in] handle         The handle.
 * \param[in] next_field_ptr Pointer to the next field.
 *
 * \return MQX_OK
 * \return MQX_INVALID_PARAMETER
 * \return MQX_INVALID_COMPONENT_HANDLE
 *
 * \see _mqx_unlink_io_component_handle
 */
_mqx_uint _mqx_link_io_component_handle
(
    _mqx_uint component,
    void     *handle,
    void    **next_field_ptr
)
{
    void   *old_handle;
    KERNEL_DATA_STRUCT_PTR kernel_data;
#if MQX_CHECK_ERRORS
    void   **prev_ptr, **next_ptr;
    uint32_t offset = (uint32_t) next_field_ptr - (uint32_t) handle;
#endif

    _GET_KERNEL_DATA(kernel_data);

#if MQX_CHECK_ERRORS
    if ((component >= MAX_IO_COMPONENTS) || (handle==NULL) || (next_field_ptr==NULL)) {
        _task_set_error(MQX_INVALID_PARAMETER);
        return(MQX_INVALID_PARAMETER);
    } /* Endif */
#endif

    _int_disable();

#if MQX_CHECK_ERRORS
    if (kernel_data->IO_COMPONENTS[component] == handle) {
        _int_enable();
        return MQX_INVALID_COMPONENT_HANDLE;
    }
    else {

        prev_ptr = kernel_data->IO_COMPONENTS[component];

        while (prev_ptr) {
            next_ptr = *((void **)((uint32_t)prev_ptr + offset));
            if (next_ptr == handle) {
                _int_enable();
                return MQX_INVALID_COMPONENT_HANDLE;
            }
            else {
                prev_ptr = next_ptr;
            }
        }
    }

#endif

    old_handle = kernel_data->IO_COMPONENTS[component];
    kernel_data->IO_COMPONENTS[component] = handle;
    *next_field_ptr = old_handle;
    _int_enable();
    return MQX_OK;
}

/*!
 * \brief Removes specified component handle.
 *
 * \param[in] component      The component number.
 * \param[in] handle         The handle to remove.
 * \param[in] next_field_ptr Pointer to the next field.
 *
 * \return MQX_OK
 * \return MQX_INVALID_PARAMETER
 *
 * \warning On failure, calls _task_set_error() to set the following task error
 * code:
 * \li MQX_INVALID_PARAMETER
 *
 * \see _mqx_link_io_component_handle
 */
_mqx_uint _mqx_unlink_io_component_handle
(
    _mqx_uint component,
    void     *handle,
    void   ** next_field_ptr
)
{
    uint32_t offset = (uint32_t) next_field_ptr - (uint32_t) handle;
    void   *prev_ptr,*next_ptr;
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _mqx_uint result = MQX_INVALID_COMPONENT_HANDLE;

    _GET_KERNEL_DATA(kernel_data);

#if MQX_CHECK_ERRORS
    if ((component >= MAX_IO_COMPONENTS) || (handle==NULL) || (next_field_ptr==NULL)) {
        _task_set_error(MQX_INVALID_PARAMETER);
        return(MQX_INVALID_PARAMETER);
    } /* Endif */
#endif

    _int_disable();

    if (kernel_data->IO_COMPONENTS[component] == handle) {
        kernel_data->IO_COMPONENTS[component] = *((void **)((uint32_t)handle + offset));
        *((void **) ((uint32_t)handle + offset)) = NULL;
        result = MQX_OK;
    }
    else {

        prev_ptr = kernel_data->IO_COMPONENTS[component];

        while (prev_ptr) {
            next_ptr = *((void **)((uint32_t)prev_ptr + offset));
            if (next_ptr == handle) {
                *((void **) ((uint32_t)prev_ptr + offset)) = *((void **)((uint32_t)next_ptr + offset));
                *((void **) ((uint32_t)next_ptr + offset)) = NULL;
                result = MQX_OK;
                break;
            }
            else {
                prev_ptr = next_ptr;
            }
        }
    }

    _int_enable();
    return result;
}
#endif

/*!
 * \private
 *
 * \brief Initialize the static parts of the kernel data structure.
 *
 * Care has to be taken when calling functions within this file, as the kernel
 * is not running yet. Specifically, functions which rely on _mqx_get_kernel_data
 * can not be called.
 */
void _mqx_init_kernel_data_internal
(
    void
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    TASK_TEMPLATE_STRUCT_PTR task_template_ptr;
    TD_STRUCT_PTR td_ptr;
    _mqx_uint priority_levels;
    _mqx_uint i;

    _GET_KERNEL_DATA(kernel_data);

    /* Store the configuration used when the kernel was compiled */
    kernel_data->CONFIG1 = MQX_CNFG1;
    kernel_data->CONFIG2 = MQX_CNFG2;

    /* Store the addressability of the processor. How many bits in a byte. */
    kernel_data->ADDRESSING_CAPABILITY = PSP_MEMORY_ADDRESSING_CAPABILITY;

    /* Indicate the endianess of the target */
    kernel_data->ENDIANESS = PSP_ENDIAN;

    /* Store PSP memory alignment information */

#if PSP_MEM_STOREBLOCK_ALIGNMENT != 0
    kernel_data->PSP_CFG_MEM_STOREBLOCK_ALIGNMENT = PSP_MEM_STOREBLOCK_ALIGNMENT;
#endif

    kernel_data->PSP_CFG_MEMORY_ALIGNMENT = PSP_MEMORY_ALIGNMENT;
    kernel_data->PSP_CFG_STACK_ALIGNMENT = PSP_STACK_ALIGNMENT;

    /*
     * Fill in fields of the kernel data structure from the initialization
     * structure.
     */
    /*kernel_data->PROCESSOR_NUMBER =  kernel_data->INIT.PROCESSOR_NUMBER;*/

    /* Set IPC id for compatibility */
#if MQX_USE_IPC
    kernel_data->MY_IPC_ID = BUILD_TASKID(kernel_data->INIT.PROCESSOR_NUMBER, 1);
#endif

    /* Store location of current interrupt vector table */
#if MQX_EXIT_ENABLED
    kernel_data->USERS_VBR = (_mqx_max_type)_int_get_vector_table();
#endif

#if MQX_CHECK_ERRORS
    if (kernel_data->INIT.TASK_TEMPLATE_LIST == NULL) {
        _mqx_exit(MQX_INVALID_POINTER);
    } /* Endif */
#endif

#if MQX_HAS_TIME_SLICE
    /* Set the default scheduling policy for created tasks */
    kernel_data->SCHED_POLICY = MQX_SCHED_FIFO;
#endif

#if MQX_KD_HAS_COUNTER
    /* Initialize the kernel counter. */
    kernel_data->COUNTER = 1U;
#endif

    /* Set up the disable and enable priority levels */
    _psp_set_kernel_disable_level();

    /*
     * Initialize the system task so that functions which update the
     * task error code can be called.
     * The system task never runs, but it's TD is used for error codes
     * during initialization, and for storage of memory blocks assigned
     * to the system.
     */
    td_ptr = (TD_STRUCT_PTR) & kernel_data->SYSTEM_TD;
    kernel_data->ACTIVE_PTR = td_ptr;
    kernel_data->ACTIVE_SR = kernel_data->DISABLE_SR;
    td_ptr->TASK_SR = kernel_data->DISABLE_SR;
    td_ptr->TASK_ID = BUILD_TASKID(kernel_data->INIT.PROCESSOR_NUMBER, SYSTEM_TASK_NUMBER);
    td_ptr->STATE = BLOCKED;

    /* Initialize the light weight semaphores queue */
    _QUEUE_INIT(&kernel_data->LWSEM, 0);

#if MQX_ENABLE_USER_MODE
    _QUEUE_INIT(&kernel_data->USR_LWSEM, 0);
#endif

#if MQX_HAS_TICK
    /* Set up the timeout queue */
    _QUEUE_INIT(&kernel_data->TIMEOUT_QUEUE, 0);
#endif

    /*
     * Compute the number of MQX priority levels needed. This is done
     * by determining the task that has the lowest priority (highest number)
     */
    priority_levels = 0;
    task_template_ptr = kernel_data->INIT.TASK_TEMPLATE_LIST;
    for (i = 0; task_template_ptr->TASK_TEMPLATE_INDEX && (i < MQX_MAXIMUM_NUMBER_OF_TASK_TEMPLATES); ++i, ++task_template_ptr) {
        if (priority_levels < task_template_ptr->TASK_PRIORITY) {
            priority_levels = task_template_ptr->TASK_PRIORITY;
        } /* Endif */
    } /* Endfor */
    kernel_data->LOWEST_TASK_PRIORITY = priority_levels;

    /*
     * Initialize the task template for the INIT Task.
     * NOTE that the idle task runs at 1 level lower than any user task.
     */
    task_template_ptr = (TASK_TEMPLATE_STRUCT_PTR)&kernel_data->INIT_TASK_TEMPLATE;
    task_template_ptr->TASK_TEMPLATE_INDEX = INIT_TASK;
    task_template_ptr->TASK_STACKSIZE      = PSP_INIT_STACK_SIZE;
    task_template_ptr->TASK_NAME           = MQX_INIT_TASK_NAME;
    task_template_ptr->TASK_ADDRESS        = _mqx_init_task;
    task_template_ptr->TASK_PRIORITY       = priority_levels + 1;

#if MQX_USE_IDLE_TASK
    /*
     * Initialize the task template for the IDLE Task.
     * NOTE that the idle task runs at 1 level lower than any user task.
     */
    task_template_ptr = (TASK_TEMPLATE_STRUCT_PTR)&kernel_data->IDLE_TASK_TEMPLATE;
    task_template_ptr->TASK_TEMPLATE_INDEX = IDLE_TASK;
    task_template_ptr->TASK_STACKSIZE      = PSP_IDLE_STACK_SIZE;
    task_template_ptr->TASK_NAME           = MQX_IDLE_TASK_NAME;
    task_template_ptr->TASK_ADDRESS        = _mqx_idle_task;
    task_template_ptr->TASK_PRIORITY       = priority_levels + 2;
#endif

    /*
     * Initialize the linked list of all TDs in the system.
     * Initially zero. Not including system TD
     */
    _QUEUE_INIT(&kernel_data->TD_LIST, 0);

    /* Set the TD counter */
    /* Start SPR P171-0014-02       */
    /* kernel_data->TD_COUNTER = 1; */
    kernel_data->TASK_NUMBER = 1;
    /* End SPR P171-0014-02         */

} /* Endbody */

/*!
 * \brief This function gets the address of the TD_STRUCT.CRT_TLS field.
 *
 * \return Adress of the TD_STRUCT.CRT_TLS field.
 * \return NULL (Local storage thread disabled or kernel_data->IN_ISR is not set.)
 */
void *_crt_tls_reference(void)
{ /* Body */
#if MQX_THREAD_LOCAL_STORAGE_ENABLE
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _GET_KERNEL_DATA(kernel_data);

    if (kernel_data->IN_ISR)
    {
        return NULL;
    } /* Endif */

    return &kernel_data->ACTIVE_PTR->CRT_TLS;
#else
    return NULL;
#endif
} /* Endbody */

#if MQX_MEM_MONITOR
_mqx_uint  _mqx_print_ram_usage()
{
#if MQX_MONITOR_STACK
    KERNEL_DATA_STRUCT_PTR kernel_data;
    TD_STRUCT_PTR          td_ptr;
    _mem_size              stack_size;
    _mem_size              stack_used;
    _mqx_uint              size;
    _mem_size              mem_allocated;
    _mem_size              pools_size;

    mem_allocated = _lwmem_highest_used();
    pools_size = _lwmem_get_size_pools();
#if MQX_USE_MEM
    pools_size += _mem_get_size_pools();
#endif

    printf("\nHeap usage: \n\n");
    printf("    SIZE :  %08ld(0x%08lx)\n", (uint32_t)pools_size,(uint32_t)pools_size);
    printf("    USED :  %08ld(0x%08lx)\n\n", (uint32_t)mem_allocated,(uint32_t)mem_allocated);

    _GET_KERNEL_DATA(kernel_data);

    /* Now display the gathered information */
    _task_int_stack_usage(&stack_size, &stack_used);
    printf("Stack usage: \n\n");
    printf("    Interrupt stack:\n");
    printf("    SIZE                  USED\n");
    printf("    %08ld(0x%08lx)    %08ld(0x%08lx)\n\n",
                    (uint32_t)stack_size,(uint32_t)stack_size, (uint32_t)stack_used,(uint32_t)stack_used);
    printf("    Task stack:\n");
    printf("    SIZE                  USED                 TASK ID      NAME\n");

    _lwsem_wait((LWSEM_STRUCT_PTR)(&kernel_data->TASK_CREATE_LWSEM));
    td_ptr = (TD_STRUCT_PTR)((unsigned char *)kernel_data->TD_LIST.NEXT -
                    FIELD_OFFSET(TD_STRUCT,TD_LIST_INFO));
    size = _QUEUE_GET_SIZE(&kernel_data->TD_LIST);

    while (size && td_ptr)
    {
        _task_stack_usage(td_ptr->TASK_ID, &stack_size, &stack_used);
        printf("    %08ld(%08lX)    %08ld(%08lX)   %08lX     %s\n",
                        (uint32_t)stack_size, (uint32_t)stack_size, (uint32_t)stack_used,
                        (uint32_t)stack_used, td_ptr->TASK_ID,
                        (td_ptr->TASK_TEMPLATE_PTR->TASK_NAME != NULL) ?
                        td_ptr->TASK_TEMPLATE_PTR->TASK_NAME : "");
        size--;
        td_ptr = (TD_STRUCT_PTR)((unsigned char *)(td_ptr->TD_LIST_INFO.NEXT) -
                        FIELD_OFFSET(TD_STRUCT,TD_LIST_INFO));
    } /* Endwhile */

    _lwsem_post((LWSEM_STRUCT_PTR)(&kernel_data->TASK_CREATE_LWSEM));

#else
    printf("Stack usage: ERROR MQX_MONITOR_STACK not set in mqx_cnfg.h\n\n");
#endif
    return MQX_OK;
}
#endif
/* EOF */
