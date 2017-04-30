/*HEADER**********************************************************************
*
* Copyright 2008-2014 Freescale Semiconductor, Inc.
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
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   Revision History:
*   Date             Version  Changes
*   ---------        -------  -------
*   Jan.20/04        2.50     Initial version
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include "fsl_flexcan_int.h"
#include "fsl_flexcan_hal.h"

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

/*!
 * @brief Returns pointer to base address of the desired CAN device.
 *
 * @param   dev_num      FlexCAN device number.
 * @return  Pointer to desired CAN device or NULL if not present.
 */
void *_bsp_get_flexcan_base_address
(
    uint8_t dev_num
)
{
    void   *addr;

    switch(dev_num) {
    case 0:
        addr = (void *)CAN0_BASE_PTR;
        break;
    case 1:
        addr = (void *)CAN1_BASE_PTR;
        break;
    default:
        addr = NULL;
    }

    return addr;
}

/*FUNCTION****************************************************************
* 
* Function Name    : _bsp_get_flexcan_vector
* Returned Value   : MQX vector number for specified interrupt
* Comments         :
*    This function returns index into MQX interrupt vector table for
*    specified flexcan interrupt. If not known, returns 0.
*
*END*********************************************************************/
/*!
 * @brief Returns index into MQX interrupt vector table for specified flexcan interrupt.
 *
 * @param   dev_num             FlexCAN device number.
 * @param   vector_type         FlexCAN interrupt vector type.
 * @param   vector_index        FlexCAN interrupt vector index.
 * @return  MQX vector number for specified interrupt. If not known, returns 0.
 */
uint32_t _bsp_get_flexcan_vector 
(
    uint8_t dev_num,
    uint8_t vector_type,
    uint32_t vector_index
)
{
    uint32_t index = (uint32_t)0;

    switch (dev_num)
    {
        case 0:
#if PSP_MQX_CPU_IS_VYBRID_A5
            index = GIC_FlexCAN0;
#elif PSP_MQX_CPU_IS_VYBRID_M4
            index = NVIC_FlexCAN0;
#else
            switch (vector_type)
            {
                case kFlexCanInt_Buf:
                     index = INT_CAN0_ORed_Message_buffer;
                     break;
                case kFlexCanInt_Err:
                     index = INT_CAN0_Error;
                     break;
                case kFlexCanInt_Boff:
                     index = INT_CAN0_Bus_Off;
                     break;
                default:
                     break;
            }
#endif
            break;
      case 1:
#if PSP_MQX_CPU_IS_VYBRID_A5
            index = GIC_FlexCAN1;
#elif PSP_MQX_CPU_IS_VYBRID_M4
            index = NVIC_FlexCAN1;
#else
         switch (vector_type)
         {
            case kFlexCanInt_Buf:
                 index = INT_CAN1_ORed_Message_buffer;
                 break;
            case kFlexCanInt_Err:
                 index = INT_CAN1_Error;
                 break;
            case kFlexCanInt_Boff:
                 index = INT_CAN1_Bus_Off;
                 break;
            default:
                 break;
         }
#endif
         break;
      default: break;
    }

    return index;
}

/*!
 * @brief Enables interrupt for requested mailbox.
 *
 * @param   dev_num                FlexCAN device number.
 * @param   mailbox_number         Mailbox number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_int_enable
(
    uint8_t dev_num,
    uint32_t mailbox_number
)
{
    volatile uint32_t     index;

    if ( mailbox_number > (CAN_MCR_MAXMB (0xFFFFFFFF)) ) 
    {
        return (kFlexCan_INVALID_MAILBOX);
    }

    index = _bsp_get_flexcan_vector(dev_num, kFlexCanInt_Buf, mailbox_number);
    if (0 == index)
    {
        return (kFlexCan_INT_ENABLE_FAILED);
    }
         
    if (_bsp_int_init(index, kFlexCan_MESSBUF_INT_LEVEL, kFlexCan_MESSBUF_INT_SUBLEVEL, TRUE) !=
        MQX_OK)
    {
        return (kFlexCan_INT_ENABLE_FAILED);
    }

    return( kFlexCan_OK );
}

/*!
 * @brief Masks (disables) interrupt for requested mailbox.
 *
 * @param   dev_num                FlexCAN device number.
 * @param   mailbox_number         Mailbox number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_int_disable
(
    uint8_t dev_num,
    uint32_t mailbox_number
)
{
    volatile uint32_t     index;

    if ( mailbox_number > (CAN_MCR_MAXMB (0xFFFFFFFF)) )
    {
        return (kFlexCan_INVALID_MAILBOX);
    }

    index = _bsp_get_flexcan_vector(dev_num, kFlexCanInt_Buf, mailbox_number);
    if (0 == index)
    {
        return (kFlexCan_INT_DISABLE_FAILED);
    }

    // Disable the interrupt
    if (_bsp_int_init(index, kFlexCan_MESSBUF_INT_LEVEL, kFlexCan_MESSBUF_INT_SUBLEVEL, FALSE) !=
        MQX_OK)
    {
        return (kFlexCan_INT_DISABLE_FAILED);
    }

    return (kFlexCan_OK);
}

/*!
 * @brief Installs interrupt handler for requested mailbox.
 *
 * @param   dev_num                FlexCAN device number.
 * @param   mailbox_number         Mailbox number.
 * @param   isr                    Interrupt service routine.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_install_isr
(
    uint8_t       dev_num,
    uint32_t      mailbox_number,
    INT_ISR_FPTR isr
)
{
    uint32_t   return_code = kFlexCan_OK;
    INT_ISR_FPTR result;
    volatile CAN_MemMapPtr                 can_reg_ptr;
    volatile uint32_t     index;

    can_reg_ptr = _bsp_get_flexcan_base_address(dev_num);
    if (NULL == can_reg_ptr)  
    {
        return (kFlexCan_INVALID_ADDRESS);
    }

    if ( mailbox_number > (CAN_MCR_MAXMB (0xFFFFFFFF)) ) 
    {
        return (kFlexCan_INVALID_MAILBOX);
    }

    index = _bsp_get_flexcan_vector(dev_num, kFlexCanInt_Buf, mailbox_number);
    if (0 == index)
    {
        return (kFlexCan_INT_INSTALL_FAILED);
    }

    // Install ISR
    result = _int_install_isr(index, isr, (void *)can_reg_ptr);
    if(result == (INT_ISR_FPTR)NULL)
    {
        return_code = _task_get_error();
    }

    return return_code;
}

/*!
 * @brief Uninstalls interrupt handler for requested mailbox.
 *
 * @param   dev_num                FlexCAN device number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_uninstall_isr
(
    uint8_t       dev_num
)
{
    uint32_t   return_code = kFlexCan_OK;
    INT_ISR_FPTR result;
    volatile uint32_t     index;

    if (NULL == _bsp_get_flexcan_base_address(dev_num))
    {
        return (kFlexCan_INVALID_ADDRESS);
    }

    index = _bsp_get_flexcan_vector(dev_num, kFlexCanInt_Buf, 0);
    if (0 == index)
    {
        return (kFlexCan_INT_INSTALL_FAILED);
    }

    /*Uninstall ISR*/
    result = _int_install_isr(index, _int_get_default_isr(), NULL);
    if(result == (INT_ISR_FPTR)NULL)
    {
        return_code = _task_get_error();
    }

    return return_code;
}

#if !(PSP_MQX_CPU_IS_VYBRID)
/*!
 * @brief Unmasks (enables) error, wake up & Bus off interrupts.
 *
 * @param   dev_num          FlexCAN device number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_error_int_enable
(
    uint8_t dev_num
)
{
    volatile uint32_t     index;

    index = _bsp_get_flexcan_vector (dev_num, kFlexCanInt_Err, 0);
    if (0 == index)
    {
        return (kFlexCan_INT_ENABLE_FAILED);
    }

    if (_bsp_int_init(index, kFlexCan_ERROR_INT_LEVEL, kFlexCan_ERROR_INT_SUBLEVEL, TRUE) !=
        MQX_OK)
    {
        return (kFlexCan_INT_ENABLE_FAILED);
    }

    index = _bsp_get_flexcan_vector (dev_num, kFlexCanInt_Boff, 0);
    if (0 == index)
    {
        return (kFlexCan_INT_ENABLE_FAILED);
    }

    if (_bsp_int_init(index, kFlexCan_BUSOFF_INT_LEVEL, kFlexCan_BUSOFF_INT_SUBLEVEL, TRUE) !=
        MQX_OK)
    {
        return (kFlexCan_INT_ENABLE_FAILED);
    }

    return ( kFlexCan_OK );
}

/*!
 * @brief Masks (disables) error, wake up & Bus off interrupts.
 *
 * @param   dev_num           FlexCAN device number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_error_int_disable
(
    uint8_t dev_num
)
{
    volatile CAN_MemMapPtr                 can_reg_ptr;
    volatile uint32_t     index;

    can_reg_ptr = _bsp_get_flexcan_base_address(dev_num);
    if (NULL == can_reg_ptr)  
    {
        return (kFlexCan_INVALID_ADDRESS);
    }

    // BOFFMSK = 0x1, ERRMSK = 0x1
    can_reg_ptr->CTRL1 &= ~(CAN_CTRL1_BOFFREC_MASK | CAN_CTRL1_ERRMSK_MASK);

    index = _bsp_get_flexcan_vector(dev_num, kFlexCanInt_Err, 0);
    if (0 == index)
    {
        return (kFlexCan_INT_DISABLE_FAILED);
    }

    if (_bsp_int_init(index, kFlexCan_ERROR_INT_LEVEL, kFlexCan_ERROR_INT_SUBLEVEL, FALSE) !=
        MQX_OK)
    {
        return (kFlexCan_INT_DISABLE_FAILED);
    }

    index = _bsp_get_flexcan_vector (dev_num, kFlexCanInt_Boff, 0);
    if (0 == index)
    {
        return (kFlexCan_INT_DISABLE_FAILED);
    }

    if (_bsp_int_init(index, kFlexCan_BUSOFF_INT_LEVEL, kFlexCan_BUSOFF_INT_SUBLEVEL, FALSE) !=
        MQX_OK)
    {
        return (kFlexCan_INT_DISABLE_FAILED);
    }

    return ( kFlexCan_OK );
}

/*!
 * @brief Installs interrupt handler for a flexcan error.
 *
 * @param   dev_num                FlexCAN device number.
 * @param   isr                    Interrupt service routine.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_install_isr_err_int
(
    uint8_t       dev_num,
    INT_ISR_FPTR isr
)
{
    uint32_t   return_code = kFlexCan_OK;
    INT_ISR_FPTR result;
    volatile CAN_MemMapPtr                 can_reg_ptr;
    volatile uint32_t     index;

    can_reg_ptr = _bsp_get_flexcan_base_address(dev_num);
    if (NULL == can_reg_ptr)  
    {
        return (kFlexCan_INVALID_ADDRESS);
    }

    index = _bsp_get_flexcan_vector(dev_num, kFlexCanInt_Err, 0);
    if (0 == index)
    {
        return (kFlexCan_INT_INSTALL_FAILED);
    }

    result = _int_install_isr(index, isr, (void *)can_reg_ptr); 
    if(result == (INT_ISR_FPTR)NULL)
    {
        return_code = _task_get_error();
    }

    return return_code;
}

/*!
 * @brief Uninstalls interrupt handler for a flexcan error.
 *
 * @param   dev_num                FlexCAN device number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_uninstall_isr_err_int
(
    uint8_t       dev_num
)
{
    uint32_t   return_code = kFlexCan_OK;
    INT_ISR_FPTR result;
    volatile uint32_t     index;

    if (NULL == _bsp_get_flexcan_base_address(dev_num))
    {
        return (kFlexCan_INVALID_ADDRESS);
    }
    index = _bsp_get_flexcan_vector(dev_num, kFlexCanInt_Err, 0);
    if (0 == index)
    {
        return (kFlexCan_INT_INSTALL_FAILED);
    }

    result = _int_install_isr(index, _int_get_default_isr(), NULL);
    if(result == (INT_ISR_FPTR)NULL)
    {
        return_code = _task_get_error();
    }

    return return_code;
}
/*!
 * @brief Installs interrupt handler for a flexcan bus off.
 *
 * @param   dev_num                FlexCAN device number.
 * @param   isr                    Interrupt service routine.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_install_isr_boff_int
(
    uint8_t       dev_num,
    INT_ISR_FPTR isr
)
{
    uint32_t   return_code = kFlexCan_OK;
    INT_ISR_FPTR result;
    volatile CAN_MemMapPtr                 can_reg_ptr;
    volatile uint32_t     index;

    can_reg_ptr = _bsp_get_flexcan_base_address(dev_num);
    if (NULL == can_reg_ptr)  
    {
        return (kFlexCan_INVALID_ADDRESS);
    }

    index = _bsp_get_flexcan_vector(dev_num, kFlexCanInt_Boff, 0);
    if (0 == index)
    {
        return (kFlexCan_INT_INSTALL_FAILED);
    }

    result = _int_install_isr(index, isr, (void *)can_reg_ptr); 
    if(result == (INT_ISR_FPTR)NULL)
    {
        return_code = _task_get_error();
    }

    return return_code;
}

/*!
 * @brief Uninstalls interrupt handler for a flexcan bus off.
 *
 * @param   dev_num                FlexCAN device number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_uninstall_isr_boff_int
(
    uint8_t       dev_num
)
{
    uint32_t   return_code = kFlexCan_OK;
    INT_ISR_FPTR result;
    volatile uint32_t     index;

    if (NULL == _bsp_get_flexcan_base_address(dev_num))
    {
        return (kFlexCan_INVALID_ADDRESS);
    }
    index = _bsp_get_flexcan_vector(dev_num, kFlexCanInt_Boff, 0);
    if (0 == index)
    {
        return (kFlexCan_INT_INSTALL_FAILED);
    }

    result = _int_install_isr(index, _int_get_default_isr(), NULL);
    if(result == (INT_ISR_FPTR)NULL)
    {
        return_code = _task_get_error();
    }

    return return_code;
}

/*!
 * @brief Installs interrupt handler for a flexcan wake-up.
 *
 * @param   dev_num                FlexCAN device number.
 * @param   isr                    Interrupt service routine.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_install_isr_wake_int
(
    uint8_t       dev_num,
    INT_ISR_FPTR isr
)
{
    uint32_t   return_code = kFlexCan_OK;
    INT_ISR_FPTR result;
    volatile CAN_MemMapPtr                 can_reg_ptr;
    volatile uint32_t     index;

    can_reg_ptr = _bsp_get_flexcan_base_address(dev_num);
    if (NULL == can_reg_ptr)  
    {
        return (kFlexCan_INVALID_ADDRESS);
    }

    index = _bsp_get_flexcan_vector(dev_num, kFlexCanInt_Wakeup, 0);
    if (0 == index)
    {
        return (kFlexCan_INT_INSTALL_FAILED);
    }

    result = _int_install_isr(index, isr, (void *)can_reg_ptr); 
    if(result == (INT_ISR_FPTR)NULL)
    {
        return_code = _task_get_error();
    }

    return return_code;
}


/*!
 * @brief Uninstalls interrupt handler for a flexcan wake-up.
 *
 * @param   dev_num                FlexCAN device number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_uninstall_isr_wake_int
(
    uint8_t       dev_num
)
{
    uint32_t   return_code = kFlexCan_OK;
    INT_ISR_FPTR result;
    volatile uint32_t     index;

    if (NULL == _bsp_get_flexcan_base_address(dev_num))
    {
        return (kFlexCan_INVALID_ADDRESS);
    }
    index = _bsp_get_flexcan_vector(dev_num, kFlexCanInt_Wakeup, 0);
    if (0 == index)
    {
        return (kFlexCan_INT_INSTALL_FAILED);
    }

    result = _int_install_isr(index, _int_get_default_isr(), NULL);
    if(result == (INT_ISR_FPTR)NULL)
    {
        return_code = _task_get_error();
    }

    return return_code;
}
#endif

