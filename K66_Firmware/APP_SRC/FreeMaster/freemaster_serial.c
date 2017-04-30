/*******************************************************************************
*
* Copyright 2004-2014 Freescale Semiconductor, Inc.
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale FreeMASTER License
* distributed with this Material.
* See the LICENSE file distributed for more details.
*
****************************************************************************//*!
*
* @brief  FreeMASTER serial communication routines
*
*******************************************************************************/

#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_protocol.h"
#include "App.h"

extern T_monitor_driver *frm_serial_drv;

/* FreeMASTER communication buffer (in/out) plus the STS and LEN bytes */
static uint8_t pcm_pCommBuffer[FMSTR_COMM_BUFFER_SIZE+3];

/* FreeMASTER runtime flags */
/*lint -e{960} using union */
typedef volatile union
{
  uint8_t all;

  struct {
    unsigned tx_active : 1;         /* response is being transmitted */
    unsigned bTxLastCharSOB   : 1;  /* last transmitted char was equal to SOB  */
    unsigned rx_SOB   : 1;  /* last received character was SOB */
    unsigned rx_next_len : 1;  /* expect the length byte next time */
    unsigned bTxFirstSobSend  : 1;  /* to send SOB char at the begin of the packet */
    unsigned bMqxReadyToSend  : 1;  /* to send next character in transmit routine */
  } flg;

} T_fm_serial_flags;

static T_fm_serial_flags fms_flags;

/* receive and transmit buffers and counters */
static uint8_t    tx_counter;       /* transmission to-do counter (0 when tx is idle) */
static uint8_t    rx_counter;       /* reception to-do counter (0 when rx is idle) */
static uint8_t *tx_buff;          /* pointer to next byte to transmit */
static uint8_t *rx_buff;          /* pointer to next free place in RX buffer */
static uint8_t    rx_csum;          /* checksum of data being received */


static void        FMSTR_Listen(void);
static void        FMSTR_SendError(uint8_t nErrCode);
static int32_t     FMSTR_init_OS_interface(void);
static void        FMSTR_Process_OS_IO(void);

/**************************************************************************//*!
*
* @brief    OS communication interface initialization
*
******************************************************************************/

static int32_t FMSTR_init_OS_interface(void)
{
  /* Open communication port */
  //devfd = fopen(FMSTR_MQX_IO_CHANNEL, (char const *)FMSTR_MQX_IO_PARAMETER);

  //return (devfd != NULL);

  return 1;
}

/*******************************************************************************
*
* @brief    Handle OS IO serial communication (both TX and RX)
*
* This function calls MQX IO fread() function to get character and process it by
*
* FMSTR_Rx function when FreeMASTER packet is receiving. This function also transmit
*
* FreeMASTER response. Character to be send is provided by call of FMSTR_Tx function
*
* and passed down to fwrite() function.
*
*******************************************************************************/

static void FMSTR_Process_OS_IO(void)
{
  static uint8_t tx_char = 0;
  /* transmitter not active, able to receive */
  if ((!fms_flags.flg.tx_active) && (!fms_flags.flg.bMqxReadyToSend))
  {
    uint8_t rx_char;
    /* read all available bytes from communication interface */
    //while (Virtual_com2_get_data(&rx_char, 2) == RESULT_OK)
    do
    {
      if (frm_serial_drv->_wait_char(&rx_char, 2) == MQX_OK) 
        if (FMSTR_Rx(rx_char)) break;
    }
    while(1);
  }

  /* transmitter active and empty? */
  if (fms_flags.flg.tx_active || fms_flags.flg.bMqxReadyToSend)
  {
    while (1)
    {
      /* write character when is valid */
      if (fms_flags.flg.bMqxReadyToSend)
      {
        if (frm_serial_drv->_send_buf(&tx_char, 1) == MQX_OK) 
          /* character was successfully send, ready to get next character */
          fms_flags.flg.bMqxReadyToSend = 0;
        else
          /* character write failed, needs to be send next call */
          break;

      }
      /* is ready to get next character? */
      if (FMSTR_Tx((uint8_t *)&tx_char))
        /* FreeMASTER packet is sent, exit loop */
        break;
      /* read next character, set its validity to be send */
      fms_flags.flg.bMqxReadyToSend = 1;
    }
  }
}

/**************************************************************************//*!
*
* @brief    Start listening on a serial line
*
* Reset the receiver machine and start listening on a serial line
*
******************************************************************************/

static void FMSTR_Listen(void)
{
  rx_counter = 0U;
  /* disable transmitter state machine */
  fms_flags.flg.tx_active = 0U;
}

/**************************************************************************//*!
*
* @brief    Send response of given error code (no data)
*
* @param    nErrCode - error code to be sent
*
******************************************************************************/

static void FMSTR_SendError(uint8_t nErrCode)
{
  /* fill & send single-byte response */
  *pcm_pCommBuffer = nErrCode;
  FMSTR_SendResponse(pcm_pCommBuffer, 1U);
}

/**************************************************************************//*!
*
* @brief    Finalize transmit buffer before transmitting
*
* @param    nLength - response length (1 for status + data length)
*
*
* This Function takes the data already prepared in the transmit buffer
* (inlcuding the status byte). It computes the check sum and kicks on tx.
*
******************************************************************************/

void FMSTR_SendResponse(uint8_t *pResponse, uint8_t nLength)
{
  uint16_t chSum = 0U;
  uint8_t i;
  uint8_t c;

  /* remember the buffer to be sent */
  tx_buff = pResponse;

  /* status byte and data are already there, compute checksum only     */
  for (i = 0U; i < nLength; i++)
  {
    c = 0U;
    pResponse = FMSTR_ValueFromBuffer8(&c, pResponse);
    /* add character to checksum */
    chSum += c;
    /* prevent saturation to happen on DSP platforms */
    chSum &= 0xffU;
  }

  /* store checksum after the message */
  pResponse = FMSTR_ValueToBuffer8(pResponse, (uint8_t)(((uint8_t)(~chSum)) + 1U));

  /* send the message and the checksum and the SOB */
  tx_counter = (uint8_t)(nLength + 1U);

  /* now transmitting the response */
  fms_flags.flg.tx_active = 1U;

  fms_flags.flg.bTxFirstSobSend = 1U;


  /* do not replicate the initial SOB  */
  fms_flags.flg.bTxLastCharSOB = 0U;


}

/**************************************************************************//*!
*
* @brief    Output buffer transmission
*
* @param  pTxChar  The character to be transmit
*
* get ready buffer(prepare data to send)
*
******************************************************************************/

int32_t FMSTR_Tx(uint8_t *pTxChar)
{
  /* to send first SOB byte*/
  if (fms_flags.flg.bTxFirstSobSend)
  {
    *pTxChar = FMSTR_SOB;
    fms_flags.flg.bTxFirstSobSend = 0U;
    return FMSTR_FALSE;
  }
  if (tx_counter)
  {
    /* fetch & send character ready to transmit */
    /*lint -e{534} ignoring return value */
    (void)FMSTR_ValueFromBuffer8(pTxChar, tx_buff);

    /* first, handle the replicated SOB characters */
    if (*pTxChar == FMSTR_SOB)
    {
      fms_flags.flg.bTxLastCharSOB ^= 1U;
      if ((fms_flags.flg.bTxLastCharSOB))
      {
        /* yes, repeat the SOB next time */
        return FMSTR_FALSE;
      }
    }
    /* no, advance tx buffer pointer */
    tx_counter--;
    tx_buff = FMSTR_SkipInBuffer(tx_buff, 1U);
    return FMSTR_FALSE;
  }

  /* start listening immediately */
  FMSTR_Listen();

  return FMSTR_TRUE;
}


/**************************************************************************//*!
*
* @brief  Handle received character
*
* @param  nRxChar  The character to be processed
*
* Handle the character received and -if the message is complete- call the
* protocol decode routine.
*
******************************************************************************/

int32_t FMSTR_Rx(uint8_t rx_char)
{
  T_fm_serial_flags *pflg = &fms_flags;

  /* first, handle the replicated SOB characters */
  if (rx_char == FMSTR_SOB)
  {
    pflg->flg.rx_SOB ^= 1;
    if (pflg->flg.rx_SOB)
    {
      /* this is either the first byte of replicated SOB or a  */
      /* real Start-of-Block mark - we will decide next time in FMSTR_Rx */
      return FMSTR_FALSE;
    }
  }

  /* we have got a common character preceded by the SOB -  */
  /* this is the command code! */
  if (pflg->flg.rx_SOB)
  {
    /* reset receiving process */
    rx_buff = pcm_pCommBuffer;
    *rx_buff++ = rx_char;

    /* start computing the checksum */
    rx_csum    = rx_char;
    rx_counter = 0U;

    /* if the standard command was received, the message length will come in next byte */
    pflg->flg.rx_next_len = 1U;

    /* fast command? */
    if (!((~rx_char) & FMSTR_FASTCMD))
    {
      /* fast command received, there will be no length information */
      pflg->flg.rx_next_len = 0U;
      /* as it is encoded in the command byte directly */
      rx_counter = (uint8_t)(((((uint8_t)rx_char) & FMSTR_FASTCMD_DATALEN_MASK) >> FMSTR_FASTCMD_DATALEN_SHIFT) + 1U);
    }

    /* command code stored & processed */
    pflg->flg.rx_SOB = 0U;
    return FMSTR_FALSE;
  }

  /* we are waiting for the length byte */
  if (pflg->flg.rx_next_len)
  {
    /* this byte, total data length and the checksum */
    rx_counter = (uint8_t)(1U + ((uint8_t)rx_char) + 1U);
    /* now read the data bytes */
    pflg->flg.rx_next_len = 0U;

  }

  /* waiting for a data byte? */
  if (rx_counter)
  {
    /* add this byte to checksum */
    rx_csum += rx_char;

    /* decrease number of expected bytes */
    rx_counter--;
    /* was it the last byte of the message (checksum)? */
    if (!rx_counter)
    {
      /* receive buffer overflow? */
      if (rx_buff == NULL)
      {
        FMSTR_SendError(FMSTR_STC_CMDTOOLONG);
      }
      /* checksum error? */
      else if ((rx_csum & 0xffU) != 0U)
      {
        FMSTR_SendError(FMSTR_STC_CMDCSERR);
      }
      /* message is okay */
      else
      {
        /* do decode now! */
        FMSTR_ProtocolDecoder(pcm_pCommBuffer);
      }

      return FMSTR_TRUE;
    }
    /* not the last character yet */
    else
    {
      /* is there still a space in the buffer? */
      if (rx_buff)
      {
        /*lint -e{946} pointer arithmetic is okay here (same array) */
        if (rx_buff < (pcm_pCommBuffer + FMSTR_COMM_BUFFER_SIZE))
        {
          /* store byte  */
          *rx_buff++ = rx_char;
        }
        /* buffer is full! */
        else
        {
          /* NULL rx pointer means buffer overflow - but we still need */
          /* to receive all message characters (for the single-wire mode) */
          /* so keep "receiving" - but throw away all characters from now */
          rx_buff = NULL;
        }
      }
    }
  }
  return FMSTR_FALSE;
}

/**************************************************************************//*!
*
* @brief    Serial communication initialization
*
******************************************************************************/

int32_t FMSTR_InitSerial(void)
{
  /* initialize all state variables */
  fms_flags.all = 0U;
  tx_counter = 0U;


  if (!FMSTR_init_OS_interface()) return FMSTR_FALSE;

  /* start listening for commands */
  FMSTR_Listen();
  return FMSTR_TRUE;
}



/*******************************************************************************
*
* @brief    API: Main "Polling" call from the application main loop
*
* This function either handles all the SCI communication (polling-only mode =
* FMSTR_POLL_DRIVEN) or decodes messages received on the background by SCI interrupt
* (short-interrupt mode = FMSTR_SHORT_INTR).
*
* In the JTAG interrupt-driven mode (both short and long), this function also checks
* if setting the JTAG RIE bit failed recently. This may happen because of the
* RIE is held low by the EONCE hardware until the EONCE is first accessed from host.
* FMSTR_Init (->FMSTR_Listen) is often called while the PC-side FreeMASTER is still
* turned off. So really, the JTAG is not enabled by this time and RIE bit is not set.
* This problem is detected (see how bJtagRIEPending is set above in FSMTR_Listen)
* and it is tried to be fixed periodically here in FMSTR_Poll.
*
*******************************************************************************/

void FMSTR_Poll(void)
{
  /* polled OS IO mode */
  FMSTR_Process_OS_IO();
}




