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
*   This file contains IEEE 1588 interface functions of the MACNET driver.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include "enet.h"
#include "enetprv.h"
#include "macnet_prv.h"
#include "macnet_1588.h"
#include <string.h>
#include <stdlib.h>

#if ENETCFG_SUPPORT_PTP

/* Global variables */
uint64_t  MACNET_PTP_seconds = 0;
bool  MACNET_PTP_set_rtc_time_flag = FALSE;

static ENET_MemMapPtr MACNET_PTP_master_addr;

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_swap8bytes
*  Returned Value : uint64_t
*  Comments       : Swap 8 bytes
*
*END*-----------------------------------------------------------------*/
static uint64_t MACNET_ptp_swap8bytes(uint64_t n)
{
	unsigned char temp[8];

    temp[7] = *((unsigned char *)&n);
    temp[6] = ((unsigned char *)&n)[1];
    temp[5] = ((unsigned char *)&n)[2];
    temp[4] = ((unsigned char *)&n)[3];
    temp[3] = ((unsigned char *)&n)[4];
    temp[2] = ((unsigned char *)&n)[5];
    temp[1] = ((unsigned char *)&n)[6];
    temp[0] = ((unsigned char *)&n)[7];
    return (*(uint64_t *)temp);
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_init_circ
*  Returned Value :
*  Comments       : Alloc the ring resource
*
*
*END*-----------------------------------------------------------------*/
static bool MACNET_ptp_init_circ(MACNET_PTP_CIRCULAR *buf)
{
    buf->DATA_BUF = (MACNET_PTP_TS_DATA *) _mem_alloc_system_zero((MACNET_PTP_DEFAULT_RX_BUF_SZ+1) *	sizeof(MACNET_PTP_TS_DATA));

    if (!buf->DATA_BUF)
        return FALSE;
    buf->FRONT = 0;
    buf->END = 0;
    buf->SIZE = (MACNET_PTP_DEFAULT_RX_BUF_SZ + 1);

    return TRUE;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_calc_index
*  Returned Value :
*  Comments       : Calculate index of the element
*
*
*END*-----------------------------------------------------------------*/
static inline uint32_t MACNET_ptp_calc_index(uint32_t size, uint32_t curr_index, uint32_t offset)
{
    return ((curr_index + offset) % size);
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_is_empty
*  Returned Value :
*  Comments       : Return TRUE if the buffer is empty
*
*
*END*-----------------------------------------------------------------*/
static bool MACNET_ptp_is_empty(MACNET_PTP_CIRCULAR *buf)
{
    return (buf->FRONT == buf->END);
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_nelems
*  Returned Value :
*  Comments       : Calculates the number of elements in the ring buffer
*
*
*END*-----------------------------------------------------------------*/
static uint32_t MACNET_ptp_nelems(MACNET_PTP_CIRCULAR *buf)
{
    const uint32_t FRONT = buf->FRONT;
    const uint32_t END = buf->END;
    const uint32_t SIZE = buf->SIZE;
    uint32_t n_items;

    if (END > FRONT)
        n_items = END - FRONT;
    else if (END < FRONT)
        n_items = SIZE - (FRONT - END);
    else
        n_items = 0;

    return n_items;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_is_full
*  Returned Value :
*  Comments       : Returns TRUE if the buffer is full
*
*
*END*-----------------------------------------------------------------*/
static bool MACNET_ptp_is_full(MACNET_PTP_CIRCULAR *buf)
{
    if (MACNET_ptp_nelems(buf) == (buf->SIZE - 1))
        return TRUE;
    else
        return FALSE;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_insert
*  Returned Value :
*  Comments       : Inserts new element into the ring buffer
*
*
*END*-----------------------------------------------------------------*/
bool MACNET_ptp_insert(MACNET_PTP_CIRCULAR *buf,
				          MACNET_PTP_TS_DATA *data)
{
    MACNET_PTP_TS_DATA *tmp;

    if (MACNET_ptp_is_full(buf))
        return FALSE;

    tmp = (buf->DATA_BUF + buf->END);

    _mem_copy(data, tmp, sizeof(MACNET_PTP_TS_DATA));
	
    buf->END = MACNET_ptp_calc_index(buf->SIZE, buf->END, 1);

    return TRUE;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_find_and_remove
*  Returned Value :
*  Comments       : Based on sequence ID and the clock ID
*                   find the element in the ring buffer and remove it
*
*END*-----------------------------------------------------------------*/
static int32_t MACNET_ptp_find_and_remove(MACNET_PTP_CIRCULAR *buf,
			                             MACNET_PTP_TS_DATA *data)
{
    uint32_t i;
    uint32_t SIZE = buf->SIZE, END = buf->END;

    if (MACNET_ptp_is_empty(buf))
        return ENET_ERROR;

    i = buf->FRONT;
    while (i != END) {
        if (((buf->DATA_BUF + i)->VERSION == data->VERSION) &&
            ((buf->DATA_BUF + i)->MESSAGE_TYPE == data->MESSAGE_TYPE)&&
         ((buf->DATA_BUF + i)->SEQ_ID == data->SEQ_ID) && 
            (0==(memcmp(((const void*)&(buf->DATA_BUF + i)->SPID[0]), (const void*)&data->SPID[0], MACNET_PTP_CLOCKID_SIZE))))
            break;
        i = MACNET_ptp_calc_index(SIZE, i, 1);
    }

    if (i == END) {
        /* buffer full ? */
        if ( MACNET_ptp_is_full(buf)) {
            /* drop one in front */
            buf->FRONT = MACNET_ptp_calc_index(SIZE, buf->FRONT, 1);
        }
        return ENET_ERROR;
    }

    data->TS.SEC = (buf->DATA_BUF + i)->TS.SEC;
    data->TS.NSEC = (buf->DATA_BUF + i)->TS.NSEC;

    buf->FRONT = MACNET_ptp_calc_index(SIZE, i, 1);

    return ENET_OK;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_init
*  Returned Value :
*  Comments       : Initialize required ring buffers
*
*
*END*-----------------------------------------------------------------*/
uint32_t MACNET_ptp_init(ENET_CONTEXT_STRUCT_PTR enet_ptr)
{
    MACNET_CONTEXT_STRUCT_PTR  macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR) enet_ptr->MAC_CONTEXT_PTR;
    MACNET_PTP_PRIVATE         *priv = (MACNET_PTP_PRIVATE_PTR)macnet_context_ptr->PTP_PRIV;

    if (!(enet_ptr->PARAM_PTR->OPTIONS & ENET_OPTION_PTP_INBAND)) {
        if(!MACNET_ptp_init_circ(&(priv->RX_TIME)))
            return FALSE;
    }

    if(!MACNET_ptp_init_circ(&(priv->TX_TIME)))
        return FALSE; 
    return TRUE;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_cleanup
*  Returned Value :
*  Comments       : Free all ring buffers
*
*
*END*-----------------------------------------------------------------*/
void MACNET_ptp_cleanup(ENET_CONTEXT_STRUCT_PTR enet_ptr)
{
    MACNET_CONTEXT_STRUCT_PTR  macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR) enet_ptr->MAC_CONTEXT_PTR;
    MACNET_PTP_PRIVATE         *priv = (MACNET_PTP_PRIVATE_PTR)macnet_context_ptr->PTP_PRIV;

    if (!(enet_ptr->PARAM_PTR->OPTIONS & ENET_OPTION_PTP_INBAND)) {
        if (priv->RX_TIME.DATA_BUF)
            _mem_free((void *)priv->RX_TIME.DATA_BUF);	
    }
    if (priv->TX_TIME.DATA_BUF)
        _mem_free((void *)priv->TX_TIME.DATA_BUF);
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_get_master_base_address
*  Returned Value :
*  Comments         :
*    This function returns pointer to base address of the MACNET device
*    in PTP master mode.
*
*END*-----------------------------------------------------------------*/
ENET_MemMapPtr MACNET_ptp_get_master_base_address(void)
{
    return MACNET_PTP_master_addr;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_set_master_base_address
*  Returned Value :
*  Comments         :
*    This function sets pointer to base address of the MACNET device
*    in PTP master mode.
*
*END*-----------------------------------------------------------------*/
void MACNET_ptp_set_master_base_address(ENET_MemMapPtr addr)
{
    MACNET_PTP_master_addr = addr;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_start
*  Returned Value :
*  Comments       : 1588 Module initialization
*
*
*END*-----------------------------------------------------------------*/
uint32_t MACNET_ptp_start(MACNET_PTP_PRIVATE *priv, uint32_t enetOption)
{
    MACNET_PTP_PRIVATE *fpp = priv;
 
#if defined(BSP_TWR_K60N512) || defined(BSP_TWR_K60D100M) || defined(BSP_TWR_K64F120M) || defined(BSP_FRD_K64F)
    /* Select the 1588 timer source clock - EXTAL clock */
    OSC_CR |= OSC_CR_ERCLKEN_MASK;
    SIM_SOPT2 = ((SIM_SOPT2) & ~SIM_SOPT2_TIMESRC_MASK) | SIM_SOPT2_TIMESRC(0x2);
#elif defined(BSP_TWR_K60F120M)|| defined (BSP_TWR_K70F120M)
/* Select the 1588 timer source clock osc0- EXTAL clock */
    OSC0_CR |= OSC_CR_ERCLKEN_MASK;
    SIM_SOPT2 = ((SIM_SOPT2) & ~SIM_SOPT2_TIMESRC_MASK)| SIM_SOPT2_TIMESRC(0x2);    
#elif defined (BSP_TWR_VF65GS10_A5) || defined(BSP_TWR_VF65GS10_M4)
    /* Select the 1588 time sampling clock - External RMII CLOCK IN/50M*/
    CCM_CSCMR2 &= ~CCM_CSCMR2_ENET_TS_CLK_SEL_MASK;
    CCM_CSCDR1 |= CCM_CSCDR1_ENET_TS_EN_MASK;             // enable ENET_TS_EN     
#elif defined(BSP_SVF522REVB_A5) || defined(BSP_SVF522REVB_M4)
    /* Select the pll5 main clock due to the RMII_REF_Clk has been configured as OUT*/
    CCM_CSCMR2 &= ~CCM_CSCMR2_ENET_TS_CLK_SEL_MASK;
    CCM_CSCMR2 |= 6 << CCM_CSCMR2_ENET_TS_CLK_SEL_SHIFT;
    CCM_CSCDR1 |= CCM_CSCDR1_ENET_TS_EN_MASK;             // enable ENET_TS_EN
#endif

    /* Enable module for starting Tmr Clock */
    fpp->MACNET_PTR->ATCR = ENET_ATCR_RESTART_MASK;
    fpp->MACNET_PTR->ATINC = MACNET_1588_CLOCK_INC;
    fpp->MACNET_PTR->ATPER = MACNET_1588_ATPER_VALUE;

#if (MK60_REV_1_0 || MK60_REV_1_1 || MK60_REV_1_2)
    /* Workaround for e2579: ENET: No support for IEEE 1588, TS_TIMER, timestamp timer overflow interrupt */
    fpp->MACNET_PTR->CHANNEL[MACNET_PTP_TIMER].TCCR = MACNET_1588_ATPER_VALUE - MACNET_1588_CLOCK_INC;
    fpp->MACNET_PTR->CHANNEL[MACNET_PTP_TIMER].TCSR = (((0x5)<<ENET_TCSR_TMODE_SHIFT)&ENET_TCSR_TMODE_MASK) |
                                                      ENET_TCSR_TIE_MASK;
    fpp->MACNET_PTR->CHANNEL[MACNET_PTP_TIMER].TCCR = MACNET_1588_ATPER_VALUE - MACNET_1588_CLOCK_INC;
#endif

    /* Period PIN & EVT bits must be set for rollover detect */
    fpp->MACNET_PTR->ATCR = ENET_ATCR_PEREN_MASK | ENET_ATCR_PINPER_MASK;

    /* Set the slave mode in case the MACNET is not handling the 1588timer */
    if(! (enetOption & ENET_OPTION_PTP_MASTER_CLK))
        fpp->MACNET_PTR->ATCR |= ENET_ATCR_SLAVE_MASK;

    /* Start counter */
    fpp->MACNET_PTR->ATCR |= ENET_ATCR_EN_MASK;
    return 0;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_stop
*  Returned Value :
*  Comments       : Cleanup routine for 1588 module.
*                   When PTP is disabled this routing is called.
*
*END*-----------------------------------------------------------------*/
void MACNET_ptp_stop(ENET_MemMapPtr macnet_ptr)
{
    macnet_ptr->ATCR = 0;
    macnet_ptr->ATCR |= ENET_ATCR_RESTART_MASK;
}
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_parse
*  Returned Value :
*  Comments       : check for 1588 ptp module.
*                   and restore the right timestamp
*
*END*-----------------------------------------------------------------*/
bool MACNET_ptp_parse(unsigned char *packet, MACNET_PTP_TS_DATA *tmp_time)
{
    uint32_t  index;
    bool  ptp_msg = FALSE;

    /*check tmp_time ptr*/
    if(!tmp_time || !packet)
    return FALSE;
   	
    /*check 802.1q vlan-tag frame */
    if(*(uint16_t *)(packet + MACNET_PTP_ETHER_PKT_TYPE_OFFS)== HOST_TO_BE_SHORT_CONST(MACNET_PACKET_8021QVLAN)){
        packet+=MACNET_VLANTAG_HEADERLEN;
    }

    /* PTP over Ethernet: Check the PTPv2 over Ethernet type (identifier) */
    if (*(uint16_t *)(packet + MACNET_PTP_ETHER_PKT_TYPE_OFFS) == HOST_TO_BE_SHORT_CONST(MACNET_PACKET_TYPE_IEEE_802_3)) {
        /* see if this is the event message*/
        if(*(uint8_t *)(packet + MACNET_PTP_ETHER_MSG_TYPE_OFFS) <= 3){
            tmp_time->VERSION = (*((uint8_t *)(packet + MACNET_PTP_ETHER_VERSION_OFFS))) & 0x0F;
            tmp_time->MESSAGE_TYPE = (*((uint8_t *)(packet + MACNET_PTP_ETHER_MSG_TYPE_OFFS))) & 0x0F;
            tmp_time->SEQ_ID = HOST_TO_BE_SHORT(*((uint16_t *)(packet + MACNET_PTP_ETHER_SEQ_ID_OFFS)));
            for(index=0;index<MACNET_PTP_CLOCKID_SIZE;index++)
                tmp_time->SPID[index] = *((uint8_t *)(packet + MACNET_PTP_ETHER_CLOCKID + index));
            ptp_msg = TRUE;
       }
    }
    /* PTP over UDP: Check if port is 319 for PTP Event, and check for UDP */
    else if (*(uint16_t *)(packet + MACNET_PTP_ETHER_PKT_TYPE_OFFS) == HOST_TO_BE_SHORT_CONST(MACNET_PACKET_TYPE_IPV4)){
        if((*(uint8_t *)(packet + MACNET_PACKET_IPVERSION_OFFS) >> 4) == MACNET_PACKET_VERSION_IPV4){
            if(((*((uint16_t *)(packet + MACNET_PTP_UDP_PORT_OFFS))) == HOST_TO_BE_SHORT_CONST(MACNET_PTP_EVNT_PORT)) && 
            (*(unsigned char *)(packet + MACNET_PTP_UDP_PKT_TYPE_OFFS) == MACNET_PACKET_TYPE_UDP)){ 
                tmp_time->VERSION = (*((uint8_t *)(packet + MACNET_PTP_UDP_VERSION_OFFS))) & 0x0F;
                tmp_time->MESSAGE_TYPE = (*((uint8_t *)(packet + MACNET_PTP_UDP_MSG_TYPE_OFFS))) & 0x0F;
                tmp_time->SEQ_ID = HOST_TO_BE_SHORT(*((uint16_t *)(packet + MACNET_PTP_UDP_SEQ_ID_OFFS)));
                for(index=0;index<MACNET_PTP_CLOCKID_SIZE;index++)
                    tmp_time->SPID[index] = *((uint8_t *)(packet + MACNET_PTP_UDP_CLOCKID + index));
                ptp_msg = TRUE;
            }
        }
    }
    else if (*(uint16_t *)(packet + MACNET_PTP_ETHER_PKT_TYPE_OFFS) == HOST_TO_BE_SHORT_CONST(MACNET_PACKET_TYPE_IPV6)){
        if((*(uint8_t *)(packet + MACNET_PACKET_IPVERSION_OFFS) >> 4) == MACNET_PACKET_VERSION_IPV6){
            if(((*((uint16_t *)(packet + MACNET_PTP_IPV6_UDP_PORT_OFFS))) == HOST_TO_BE_SHORT_CONST(MACNET_PTP_EVNT_PORT)) && 
            (*(unsigned char *)(packet + MACNET_PTP_IPV6_UDP_PKT_TYPE_OFFS) == MACNET_PACKET_TYPE_UDP)){ 
                tmp_time->VERSION = (*((uint8_t *)(packet + MACNET_PTP_IPV6_UDP_VERSION_OFFS))) & 0x0F;
                tmp_time->MESSAGE_TYPE = (*((uint8_t *)(packet + MACNET_PTP_IPV6_UDP_MSG_TYPE_OFFS))) & 0x0F;
                tmp_time->SEQ_ID = HOST_TO_BE_SHORT(*((uint16_t *)(packet + MACNET_PTP_IPV6_UDP_SEQ_ID_OFFS)));
                for(index=0;index<MACNET_PTP_CLOCKID_SIZE;index++)
                    tmp_time->SPID[index] = *((uint8_t *)(packet + MACNET_PTP_IPV6_UDP_CLOCKID + index));
                ptp_msg = TRUE;
            }  
        } 
    }
    return ptp_msg;
}



/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_get_curr_cnt
*  Returned Value :
*  Comments       : Gets current value of the precise 1588 counter
*
*
*END*-----------------------------------------------------------------*/
static void MACNET_ptp_get_curr_cnt(ENET_MemMapPtr macnet_ptr,
                                    MACNET_PTP_TIME *curr_time)
{
    MACNET_int_disable();
    curr_time->SEC = MACNET_PTP_seconds;
    /* To read the current value, issue a capture command (set
       ENETn_ATCR[CAPTURE]) prior to reading this register */
    macnet_ptr->ATCR |= ENET_ATCR_CAPTURE_MASK;
    macnet_ptr->ATCR |= ENET_ATCR_CAPTURE_MASK;
    curr_time->NSEC = macnet_ptr->ATVR;
    MACNET_int_enable();
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_set_1588cnt
*  Returned Value :
*  Comments       : Set the precise 1588 counter
*
*
*END*-----------------------------------------------------------------*/
static void MACNET_ptp_set_1588cnt(ENET_MemMapPtr macnet_ptr,
			                       MACNET_PTP_TIME *fec_time)
{
    MACNET_int_disable();
    MACNET_PTP_seconds = fec_time->SEC;
    macnet_ptr->ATVR = fec_time->NSEC;
    MACNET_PTP_set_rtc_time_flag = TRUE;
    MACNET_int_enable();
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_set_correction
*  Returned Value :
*  Comments       : Sets 1588counter corrections
*                   to eliminate the clock drift
*
*END*-----------------------------------------------------------------*/
static void MACNET_ptp_set_correction(MACNET_PTP_PRIVATE *priv, int32_t drift)
{
    uint32_t atinc_tmp,min_inc;
    uint32_t corr_perid,count=0;
    uint8_t   inc;

    if(drift == 0)
    {
        /* no need to do correction*/
        min_inc = 0;
        corr_perid = 0;
    }

    else if(abs(drift) >= MACNET_1588_CLOCK_SRC)
    {
       /* Drift is greater than the 1588 source clock; 
          the correction increment should be applied every tic of the 1588 timer 
          to speed up/slow very rapidly */
       min_inc = (uint32_t)(abs(drift)/MACNET_1588_CLOCK_SRC);
       corr_perid = 1;
    }
    else
    {
       /* choose to do correction each corr_period larger than one ticket*/
       for(count=1; count <= MACNET_1588_CLOCK_INC; count++)
       {
           if((count * MACNET_1588_ATPER_VALUE)/abs(drift) >  MACNET_1588_CLOCK_INC)
           {
               min_inc = count;
               corr_perid = (min_inc * MACNET_1588_ATPER_VALUE)/(abs(drift) * MACNET_1588_CLOCK_INC);
               break;
           }
       } 
    }
  
    /* Clock drift is reset */
    if (drift < 0){
        inc = MACNET_1588_CLOCK_INC - min_inc;
    }else{
        inc = MACNET_1588_CLOCK_INC + min_inc;
    }

    priv->MACNET_PTR->ATCOR  = ENET_ATCOR_COR(corr_perid);
    if(inc>(ENET_ATINC_INC_CORR_MASK>>ENET_ATINC_INC_CORR_SHIFT)) 
    inc = (ENET_ATINC_INC_CORR_MASK>>ENET_ATINC_INC_CORR_SHIFT);

    atinc_tmp = (priv->MACNET_PTR->ATINC) & (~ENET_ATINC_INC_CORR_MASK);
    priv->MACNET_PTR->ATINC = atinc_tmp | ENET_ATINC_INC_CORR(inc);                                                                                    
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_store_txstamp
*  Returned Value :
*  Comments       : This interrupt service routine stores the captured
*                   tx timestamp in the MACNET context structure
*
*END*-----------------------------------------------------------------*/
uint32_t MACNET_ptp_store_txstamp
   (
         /* [IN] the Ethernet state structure */
      void    *enet
   )
{ /* Body */

    ENET_CONTEXT_STRUCT_PTR    enet_ptr = (ENET_CONTEXT_STRUCT_PTR)enet;
    MACNET_CONTEXT_STRUCT_PTR  macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR) enet_ptr->MAC_CONTEXT_PTR;
    ENET_MemMapPtr             macnet_ptr= macnet_context_ptr->MACNET_ADDRESS;
    bool          master_macnet_timer_overflow_flag;
    ENET_MemMapPtr             master_macnet_ptr;
    MACNET_PTP_TIME            curr_time;

    if (macnet_ptr == NULL){     
        return ENETERR_INVALID_DEVICE;
    }
    if (!(macnet_ptr->EIR & ENET_EIR_TS_AVAIL_MASK)){
        return ENETERR_NO_VALID_TXTS;
    }

    /* Clear the TS_AVAIL interrupt */
    macnet_ptr->EIR = ENET_EIR_TS_AVAIL_MASK;

    /* Store nanoseconds */
    macnet_context_ptr->PTP_PRIV->TXSTAMP.NSEC = macnet_ptr->ATSTMP;

    /* Store seconds, correct/avoid seconds incrementation when timestamp
       captured at the edge between two seconds (nanoseconds counter overflow) */
    if (MACNET_PTP_set_rtc_time_flag == FALSE) {
        master_macnet_ptr = MACNET_ptp_get_master_base_address();
        MACNET_int_disable();
        MACNET_ptp_get_curr_cnt(master_macnet_ptr, &curr_time);
#if (MK60_REV_1_0 || MK60_REV_1_1 || MK60_REV_1_2)
        master_macnet_timer_overflow_flag = (master_macnet_ptr->CHANNEL[MACNET_PTP_TIMER].TCSR) & ENET_TCSR_TF_MASK;
#else
        master_macnet_timer_overflow_flag = (master_macnet_ptr->EIR) & ENET_EIR_TS_TIMER_MASK;
#endif
        if(curr_time.NSEC > macnet_context_ptr->PTP_PRIV->TXSTAMP.NSEC)
            macnet_context_ptr->PTP_PRIV->TXSTAMP.SEC  = MACNET_PTP_seconds;
        else if(master_macnet_timer_overflow_flag)
            macnet_context_ptr->PTP_PRIV->TXSTAMP.SEC  = MACNET_PTP_seconds;
        else
            macnet_context_ptr->PTP_PRIV->TXSTAMP.SEC  = MACNET_PTP_seconds - 1;
        MACNET_int_enable();
    } else {
        /* Do not check nanoseconds counter overflow in case MACNET_PTP_SET_RTC_TIME ioctl was called */
        macnet_context_ptr->PTP_PRIV->TXSTAMP.SEC  = MACNET_PTP_seconds;
        MACNET_PTP_set_rtc_time_flag = FALSE;
    }

	return ENET_OK;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_store_rxstamp
*  Returned Value :
*  Comments       : This routine is called from the MACNET RX ISR
*                   to store the captured rx timestamp
*                   in the adequate ring buffer
*END*-----------------------------------------------------------------*/
void MACNET_ptp_store_rxstamp(ENET_CONTEXT_STRUCT_PTR enet_ptr, PCB_PTR pcb_ptr, VENET_BD_STRUCT_PTR bdp)
{
	MACNET_CONTEXT_STRUCT_PTR  macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR) enet_ptr->MAC_CONTEXT_PTR;
	MACNET_PTP_PRIVATE         *priv = (MACNET_PTP_PRIVATE_PTR)macnet_context_ptr->PTP_PRIV;
	MACNET_PTP_TS_DATA         tmp_rx_time;
	bool                    ptp_msg_received = FALSE, master_macnet_timer_overflow_flag;
    ENET_MemMapPtr             master_macnet_ptr;
    MACNET_PTP_TIME            curr_time;
    unsigned char                  *skb = pcb_ptr->FRAG[0].FRAGMENT;
    uint16_t                    chksum, len, num_added_bytes = HOST_TO_BE_SHORT_CONST(sizeof(MACNET_PTP_TIME));

    /* Check ptp message*/
    ptp_msg_received = MACNET_ptp_parse(skb,&tmp_rx_time);
    if(ptp_msg_received == TRUE) {

        /* Store nanoseconds */
        tmp_rx_time.TS.NSEC = LONG_BE_TO_HOST((uint32_t)(bdp->TIMESTAMP));

        /* Store seconds, correct/avoid seconds incrementation when timestamp
           captured at the edge between two seconds (nanoseconds counter overflow) */
        if(MACNET_PTP_set_rtc_time_flag == FALSE) {
            master_macnet_ptr = MACNET_ptp_get_master_base_address();
            MACNET_int_disable();
            MACNET_ptp_get_curr_cnt(master_macnet_ptr, &curr_time);
#if (MK60_REV_1_0 || MK60_REV_1_1 || MK60_REV_1_2)
            master_macnet_timer_overflow_flag = (master_macnet_ptr->CHANNEL[MACNET_PTP_TIMER].TCSR) & ENET_TCSR_TF_MASK;
#else
            master_macnet_timer_overflow_flag = (master_macnet_ptr->EIR) & ENET_EIR_TS_TIMER_MASK;
#endif
            if(curr_time.NSEC > tmp_rx_time.TS.NSEC)
                tmp_rx_time.TS.SEC = MACNET_PTP_seconds;
            else if(master_macnet_timer_overflow_flag) {
                tmp_rx_time.TS.SEC = MACNET_PTP_seconds;
            }
            else {
                tmp_rx_time.TS.SEC = MACNET_PTP_seconds - 1;
            }
            MACNET_int_enable();
        } else {
            /* Do not check nanoseconds counter overflow in case MACNET_PTP_SET_RTC_TIME ioctl was called */
            tmp_rx_time.TS.SEC = MACNET_PTP_seconds;
            MACNET_PTP_set_rtc_time_flag = FALSE;
        }

        /* In INBAND mode, append the timestamp to the packet */ 
        if(enet_ptr->PARAM_PTR->OPTIONS & ENET_OPTION_PTP_INBAND) {
            /* Enlarge the msg. */
            pcb_ptr->FRAG[0].LENGTH += sizeof(MACNET_PTP_TIME);

            /* Copy timestamp at the end of the msg. */
#if (PSP_ENDIAN == MQX_BIG_ENDIAN)
            _mem_copy( &tmp_rx_time.TS, (void *)(pcb_ptr->FRAG[0].FRAGMENT + MACNET_PTP_EVENT_MSG_FRAME_SIZE + MACNET_PTP_INBAND_SEC_OFFS), sizeof(MACNET_PTP_TIME) );
#else
            *(uint64_t *)(pcb_ptr->FRAG[0].FRAGMENT + MACNET_PTP_EVENT_MSG_FRAME_SIZE + MACNET_PTP_INBAND_SEC_OFFS) = MACNET_ptp_swap8bytes(tmp_rx_time.TS.SEC);
            *(uint32_t *)(pcb_ptr->FRAG[0].FRAGMENT + MACNET_PTP_EVENT_MSG_FRAME_SIZE + MACNET_PTP_INBAND_NANOSEC_OFFS	) = HOST_TO_BE_LONG(tmp_rx_time.TS.NSEC);
#endif
            /* Change the lenght field in the IP header */
            len = HOST_TO_BE_SHORT(*(uint16_t *)(skb + MACNET_PTP_UDP_IPLENGHT_OFFS));
            len += sizeof(MACNET_PTP_TIME);
            *(uint16_t *)(skb + MACNET_PTP_UDP_IPLENGHT_OFFS) = HOST_TO_BE_SHORT(len);
            /* Correct the IP header checksum */
            chksum = HOST_TO_BE_SHORT(*(uint16_t *)(skb + MACNET_PTP_UDP_IPCHECKSUM_OFFS));
            chksum = (((chksum) == 0xFFFF) ? (chksum) : ~(chksum) & 0xFFFF);
            chksum = _mem_sum_ip(chksum, sizeof(uint16_t), &num_added_bytes);
            chksum = (((chksum) == 0xFFFF) ? (chksum) : ~(chksum) & 0xFFFF);
            *(uint16_t *)(skb + MACNET_PTP_UDP_IPCHECKSUM_OFFS) = HOST_TO_BE_SHORT(chksum);

            /* Change the lenght field in the UDP header */
            len = HOST_TO_BE_SHORT(*(uint16_t *)(skb + MACNET_PTP_UDP_UDPLENGHT_OFFS));
            len += sizeof(MACNET_PTP_TIME);
            *(uint16_t *)(skb + MACNET_PTP_UDP_UDPLENGHT_OFFS) = HOST_TO_BE_SHORT(len);
            /* Correct the UDP header checksum */
            chksum = HOST_TO_BE_SHORT(*(uint16_t *)(skb + MACNET_PTP_UDP_UDPCHECKSUM_OFFS));
            chksum = (((chksum) == 0xFFFF) ? (chksum) : ~(chksum) & 0xFFFF);
            chksum = _mem_sum_ip(chksum, sizeof(MACNET_PTP_TIME), (pcb_ptr->FRAG[0].FRAGMENT + MACNET_PTP_EVENT_MSG_FRAME_SIZE + MACNET_PTP_INBAND_SEC_OFFS));
            chksum = _mem_sum_ip(chksum, sizeof(uint16_t), &num_added_bytes);
            chksum = _mem_sum_ip(chksum, sizeof(uint16_t), &num_added_bytes);
            chksum = (((chksum) == 0xFFFF) ? (chksum) : ~(chksum) & 0xFFFF);
            *(uint16_t *)(skb + MACNET_PTP_UDP_UDPCHECKSUM_OFFS) = HOST_TO_BE_SHORT(chksum);
        } else {
        /* In OUTBAND mode, store the captured rx timestamp in the adequate ring buffer */
        MACNET_ptp_insert(&(priv->RX_TIME), &tmp_rx_time);		
        }
    }
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name  : MACNET_ptp_L2queue_init
* Returned Value : -
* Comments       : Initialize the queue for PTP-Ethernet packets
*
*END*-----------------------------------------------------------------*/
static uint32_t MACNET_ptp_L2queue_init(MACNET_PTP_L2QUEUE *ps_queue)
{
    uint32_t slot;
    /* Input param check*/
    if(!ps_queue)
        return ENETERR_1588L2_ALLOC;
    ps_queue->WR_IDX = 0;
    ps_queue->RD_IDX = 0;
    for (slot = 0; slot < MACNET_PTP_DEFAULT_L2PCK_BUF_SZ; slot ++) {
        ps_queue->L2PCK[slot].LENGTH = 0;
    }

    return ENET_OK;
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name  : MACNET_ptp_L2queue_is_empty
* Returned Value : -
* Comments       : Checks if the queue is empty
*
*END*-----------------------------------------------------------------*/
static bool MACNET_ptp_L2queue_is_empty(const MACNET_PTP_L2QUEUE *ps_queue)
{
    if (ps_queue->WR_IDX == ps_queue->RD_IDX) {
        return TRUE;
    }
    return FALSE;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name  : MACNET_ptp_L2queue_add_packet
* Returned Value : -
* Comments       : Insert the PTP-Ethernet packet to the queue
*
*END*-----------------------------------------------------------------*/
static void MACNET_ptp_L2queue_add_packet(MACNET_PTP_L2QUEUE *ps_queue,
                                          const uint8_t *pb_buf,
                                          uint16_t len)
{
    if (ps_queue->L2PCK[ps_queue->WR_IDX].LENGTH != 0) {
      /* No free slots in the queue */
        return;
    }

    /* Store the packet */
    ps_queue->L2PCK[ps_queue->WR_IDX].LENGTH = len;
    _mem_copy((void *)pb_buf, (void *)ps_queue->L2PCK[ps_queue->WR_IDX].PACKET, len);

    /* Get the next queue slot to write */
    ps_queue->WR_IDX = (ps_queue->WR_IDX + 1) % MACNET_PTP_DEFAULT_L2PCK_BUF_SZ;

    return;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name  : MACNET_ptp_L2queue_get_packet
* Returned Value : -
* Comments       : Gets the PTP-Ethernet packet from the queue
*
*END*-----------------------------------------------------------------*/
static bool MACNET_ptp_L2queue_get_packet(MACNET_PTP_L2QUEUE *ps_queue,
                                             uint8_t  *pb_buf,
                                             uint16_t *pLENGTH)
{
    bool ret_val = FALSE;

    /* Check queue argument */
    if( ps_queue == NULL ) {
        return FALSE;
    }

    /* Disable MACNET interrupts in order to avoid buffer data overwrite */
    MACNET_int_disable();

    /* check queue empty */
    if(MACNET_ptp_L2queue_is_empty(ps_queue)) {
        ret_val = FALSE;
    } else {
        /* Get the packet */
        *pLENGTH = ps_queue->L2PCK[ps_queue->RD_IDX].LENGTH;
        _mem_copy((void *)ps_queue->L2PCK[ps_queue->RD_IDX].PACKET, (void *)pb_buf, *pLENGTH);

        /* Clear the queue slot */
        ps_queue->L2PCK[ps_queue->RD_IDX].LENGTH = 0;

        /* Get the next queue slot to read */
        ps_queue->RD_IDX = (ps_queue->RD_IDX + 1) % MACNET_PTP_DEFAULT_L2PCK_BUF_SZ;

        ret_val = TRUE;
    }

    MACNET_int_enable();

    return ret_val;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name  : MACNET_ptp_free_pcb
* Returned Value : -
* Comments       : This function enqueues the PCB allocated when seding
*                  layer2 packets (0x88F7 identifier).
*
*END*-----------------------------------------------------------------*/
static void MACNET_ptp_free_pcb
   (
         /* [IN] the PCB to enqueue */
      PCB_PTR  pcb_ptr
   )
{
    _mem_free((void *)pcb_ptr);
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name  : MACNET_ptp_service_L2pckts
* Returned Value : -
* Comments       : This is the callback function for Ethernet 1588
*                  layer2 packets (0x88F7 identifier). It adds the
*                  PTP-Ethernet packet to the queue.
*
*END*-----------------------------------------------------------------*/
void MACNET_ptp_service_L2pckts
   (
      PCB_PTR  pcb,
         /* [IN] the received packet */
      void    *handle
         /* [IN] the IP interface structure */
   )
{
    ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR) handle;
    MACNET_CONTEXT_STRUCT_PTR  macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR)enet_ptr->MAC_CONTEXT_PTR;
    MACNET_PTP_PRIVATE_PTR priv = macnet_context_ptr->PTP_PRIV;

    /* Add real data without ethernet header to the layer2 packet queue*/
    MACNET_ptp_L2queue_add_packet(priv->L2PCKS_PTR, pcb->FRAG[0].FRAGMENT, pcb->FRAG[0].LENGTH);

    PCB_free(pcb);
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name  : MACNET_ptp_send_L2pckts
* Returned Value : -
* Comments       : Sends PTP-Ethernet packets.
*
*END*-----------------------------------------------------------------*/
int32_t MACNET_ptp_send_L2pckts(_enet_handle handle, void *param_ptr)
{
    PCB_PTR     pcb_ptr;
    unsigned char   *mem_ptr;
    ENET_HEADER_PTR  packet_ptr;
    uint16_t     len, headerlen;
    unsigned char *typePtr;

    len = ((MACNET_PTP_ETHERTYPE_PCK *)param_ptr)->LENGTH;
    headerlen = sizeof(ENET_HEADER);
#if RTCSCFG_ENABLE_8021Q
    if(((MACNET_PTP_ETHERTYPE_PCK *)param_ptr)->vlanEnabled){
        headerlen = headerlen + MACNET_VLANTAG_HEADERLEN;
    }
#endif
    /* Allocate memory */
    mem_ptr = _mem_alloc_system_zero(sizeof(PCB)+sizeof(PCB_FRAGMENT)+ headerlen +len);
    if (mem_ptr==NULL) {
       return (ENETERR_ALLOC_PCB);
    }
    pcb_ptr = (PCB_PTR) mem_ptr;
    packet_ptr = (ENET_HEADER_PTR)&mem_ptr[sizeof(PCB)+sizeof(PCB_FRAGMENT)];

    /* Ethernet msg. header */
    htone(packet_ptr->DEST, ((MACNET_PTP_ETHERTYPE_PCK *)param_ptr)->DEST_MAC);
    htone(packet_ptr->SOURCE, ((ENET_CONTEXT_STRUCT_PTR)handle)->ADDRESS);
    typePtr = packet_ptr->TYPE;
	
#if RTCSCFG_ENABLE_8021Q 
    if(((MACNET_PTP_ETHERTYPE_PCK *)param_ptr)->vlanEnabled){

      ENET_8021QTAG_HEADER_PTR tagPtr = (ENET_8021QTAG_HEADER_PTR)(typePtr + 2);
      uint16_t vlanTag;
      
      /* Two bytes tag protocol identifier (TPID) are set to a value of 0x8100 
         in order to identify the frame as an IEEE 802.1Q-tagged frame */  
      mqx_htons(typePtr, ENETPROT_8021Q);
      /* Two bytes of tag control information (TCI). The TCI field is further divided into PCP, DEI, and VID*/
      vlanTag = (((MACNET_PTP_ETHERTYPE_PCK *)param_ptr)->vlanPrior << 13) | ((MACNET_PTP_ETHERTYPE_PCK *)param_ptr)->vlanId;  
      mqx_htons(tagPtr->TAG, vlanTag);
      typePtr = tagPtr->TYPE;
    }
#endif
     mqx_htons(typePtr, MQX1588_PTP_ETHERTYPE_1588);

    /* Payload */
    _mem_copy( (void *)(((MACNET_PTP_ETHERTYPE_PCK *)param_ptr)->PTP_MSG), (void *)&mem_ptr[sizeof(PCB)+sizeof(PCB_FRAGMENT)+ headerlen], len );

    pcb_ptr->FREE = MACNET_ptp_free_pcb;
    pcb_ptr->FRAG[0].LENGTH = headerlen + len;
    pcb_ptr->FRAG[0].FRAGMENT = (unsigned char *)packet_ptr;

    return(ENET_send_raw((ENET_CONTEXT_STRUCT_PTR)handle, pcb_ptr));
}
/*FUNCTION*-------------------------------------------------------------
*
* Function Name  : MACNET_ptp_recv_L2pckts
* Returned Value : -
* Comments       : Withdraw the PTP-Ethernet packets from the queue.
*
*END*-----------------------------------------------------------------*/
int32_t MACNET_ptp_recv_L2pckts(_enet_handle handle, void *param_ptr)
{
    ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR) handle;
    MACNET_CONTEXT_STRUCT_PTR  macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR)enet_ptr->MAC_CONTEXT_PTR;
    MACNET_PTP_PRIVATE_PTR priv = macnet_context_ptr->PTP_PRIV;


    if (FALSE == MACNET_ptp_L2queue_get_packet(priv->L2PCKS_PTR,
                                               ((MACNET_PTP_ETHERTYPE_PCK *)param_ptr)->PTP_MSG,
                                               &((MACNET_PTP_ETHERTYPE_PCK *)param_ptr)->LENGTH)) {
        return ENET_ERROR;
    }
    eaddrcpy(((MACNET_PTP_ETHERTYPE_PCK *)param_ptr)->DEST_MAC, enet_ptr->ADDRESS);
    return ENET_OK;
}


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_ptp_ioctl
*  Returned Value :
*  Comments       :
*
*
*END*-----------------------------------------------------------------*/
uint32_t MACNET_ptp_ioctl(ENET_CONTEXT_STRUCT_PTR enet_ptr, uint32_t command_id, void *inout_param)
{
    MACNET_CONTEXT_STRUCT_PTR  macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR) enet_ptr->MAC_CONTEXT_PTR;
    MACNET_PTP_PRIVATE *priv = macnet_context_ptr->PTP_PRIV;
    MACNET_PTP_TIME *cnt;
    MACNET_PTP_TIME curr_time;
    ENET_MemMapPtr master_macnet_ptr;
    int32_t retval = ENET_OK;

    master_macnet_ptr = MACNET_ptp_get_master_base_address();
    switch (command_id) {

    case MACNET_PTP_GET_RX_TIMESTAMP:
        /* Do not read rx timestamps from buffers in the inbound mode */
        if(enet_ptr->PARAM_PTR->OPTIONS & ENET_OPTION_PTP_INBAND) {
            return ENET_ERROR;
        }
		
        retval = MACNET_ptp_find_and_remove(&(priv->RX_TIME), (MACNET_PTP_TS_DATA *)inout_param);

    break;

    case MACNET_PTP_GET_TX_TIMESTAMP:
        retval = MACNET_ptp_find_and_remove(&(priv->TX_TIME), (MACNET_PTP_TS_DATA *)inout_param);
    break;

    case MACNET_PTP_GET_CURRENT_TIME:
        MACNET_ptp_get_curr_cnt(master_macnet_ptr, &curr_time);
        ((MACNET_PTP_RTC_TIME *)inout_param)->RTC_TIME = curr_time;
    break;

    case MACNET_PTP_SET_RTC_TIME:
        cnt = &(((MACNET_PTP_RTC_TIME *)inout_param)->RTC_TIME);
        MACNET_ptp_set_1588cnt(master_macnet_ptr, cnt);
    break;

    case MACNET_PTP_FLUSH_TIMESTAMP:
        /* reset sync buffer */
        priv->RX_TIME.FRONT = 0;
        priv->RX_TIME.END = 0;
        priv->RX_TIME.SIZE = (MACNET_PTP_DEFAULT_RX_BUF_SZ + 1);		
    break;

    case MACNET_PTP_SET_COMPENSATION:
        MACNET_ptp_set_correction(priv, ((MACNET_PTP_SET_COMP *)inout_param)->DRIFT);
    break;

    case MACNET_PTP_GET_ORIG_COMP:
    break;

    case MACNET_PTP_REGISTER_ETHERTYPE_PTPV2:
        /* Registers the PTPV2 protocol type on an Ethernet channel */
        retval = ENET_open(enet_ptr, MQX1588_PTP_ETHERTYPE_1588, MACNET_ptp_service_L2pckts, enet_ptr);
        if(retval == ENET_OK)
        {   /* Allocate the queue for this type of messages */
            priv->L2PCKS_PTR = (MACNET_PTP_L2QUEUE_PTR) _mem_alloc_system_zero(sizeof(MACNET_PTP_L2QUEUE));
            if(!priv->L2PCKS_PTR)
                return IO_ERROR_DEVICE_INVALID;
            retval = MACNET_ptp_L2queue_init(priv->L2PCKS_PTR);
        }
    break;

    case MACNET_PTP_UNREGISTER_ETHERTYPE_PTPV2:
        /* Un-registers the PTPV2 protocol type on an Ethernet channel */
        retval = ENET_close(enet_ptr, MQX1588_PTP_ETHERTYPE_1588);
        /* Free the queue for this type of messages */
        _mem_free(priv->L2PCKS_PTR);
    break;

    case MACNET_PTP_SEND_ETHERTYPE_PTPV2_PCK:
        retval = MACNET_ptp_send_L2pckts(enet_ptr, inout_param);
    break;

    case MACNET_PTP_RECV_ETHERTYPE_PTPV2_PCK:
        retval = MACNET_ptp_recv_L2pckts(enet_ptr, inout_param);
    break;
    case MACNET_PTP_IS_IN_INBAND_MODE:
        if(enet_ptr->PARAM_PTR->OPTIONS & ENET_OPTION_PTP_INBAND) {
            *(uint8_t*)inout_param = TRUE;
        } else {
            *(uint8_t*)inout_param = FALSE;
        }
    break;
    default:
        return IO_ERROR_INVALID_IOCTL_CMD;
    }
    return retval;
}
#endif /* ENETCFG_SUPPORT_PTP */
