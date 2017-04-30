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
* @brief  FreeMASTER protocol handler
*
*******************************************************************************/

#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_protocol.h"

#if !(FMSTR_DISABLE)

  #if FMSTR_DEBUG_TX
/* This warning is for you not to forget to disable the DEBUG_TX mode once the
 * communication line works fine. You can ignore this warning if it is okay
 * that the application will be periodically sending test frames to the world. */
    #warning FMSTR_DEBUG_TX is enabled. Test frames will be transmitted periodically.
/* When communication debugging mode is requested, this global variable is used to turn the debugging off once a valid connection is detected */
int32_t pcm_bDebugTx;
  #endif

/**************************************************************************//*!
*
* @brief    FreeMASTER driver initialization
*
******************************************************************************/

int32_t FMSTR_Init(void)
{
  #if FMSTR_USE_TSA
  /* initialize TSA */
  FMSTR_InitTsa();
  #endif

  #if FMSTR_USE_SCOPE
  /* initialize Scope */
  FMSTR_InitScope();
  #endif

  #if FMSTR_USE_RECORDER
  /* initialize Recorder */
  FMSTR_InitRec();
  #endif

  #if FMSTR_USE_APPCMD
  /* initialize application commands */
  FMSTR_InitAppCmds();
  #endif

  #if FMSTR_USE_SFIO
  /* initialize SFIO encapsulation layer */
  FMSTR_InitSfio();
  #endif

#if FMSTR_USE_PIPES
    /* initialize PIPES interface */
    FMSTR_InitPipes();
#endif




  #if FMSTR_USE_SERIAL
  /* initialize communication and start listening for commands */
  if (!FMSTR_InitSerial()) return FMSTR_FALSE;
  #endif


  return FMSTR_TRUE;
}

/**************************************************************************//*!
*
* @brief    Decodes the FreeMASTER protocol and calls appropriate handlers
*
* @param    pMessageIO - message in/out buffer
*
* @return   TRUE if frame was valid and any output was generated to IO buffer
*
* This Function decodes given message and invokes proper command handler
* which fills in the response. The response transmission is initiated
* in this call as well.
*
******************************************************************************/

int32_t FMSTR_ProtocolDecoder(uint8_t *msg)
{
  uint8_t *p_resp;
  uint8_t cmd;

  /* no EX access by default */
  FMSTR_SetExAddr(FMSTR_FALSE);

  /* command code comes first in the message */
  /*lint -e{534} return value is not used */
  (void)FMSTR_ValueFromBuffer8(&cmd, msg);

  /* process command   */
  switch (cmd)
  {

  #if FMSTR_USE_READVAR

    /* read byte */
  case FMSTR_CMD_READVAR8_EX:
    FMSTR_SetExAddr(FMSTR_TRUE);
    p_resp = FMSTR_ReadVar(msg, 1U);
    break;

    /* read word */
  case FMSTR_CMD_READVAR16_EX:
    FMSTR_SetExAddr(FMSTR_TRUE);
    p_resp = FMSTR_ReadVar(msg, 2U);
    break;

    /* read dword */
  case FMSTR_CMD_READVAR32_EX:
    FMSTR_SetExAddr(FMSTR_TRUE);
    p_resp = FMSTR_ReadVar(msg, 4U);
    break;
  #endif /* FMSTR_USE_READVAR */

  #if FMSTR_USE_READMEM

    /* read a block of memory */
  case FMSTR_CMD_READMEM_EX:
    FMSTR_SetExAddr(FMSTR_TRUE);
    p_resp = FMSTR_ReadMem(msg);
    break;

  #endif /* FMSTR_USE_READMEM */

  #if FMSTR_USE_SCOPE

    /* prepare scope variables */
  case FMSTR_CMD_SETUPSCOPE_EX:
    FMSTR_SetExAddr(FMSTR_TRUE);
    p_resp = FMSTR_SetUpScope(msg);
    break;

  case FMSTR_CMD_READSCOPE:
    p_resp = FMSTR_ReadScope(msg);
    break;
  #endif /* FMSTR_USE_SCOPE */

  #if FMSTR_USE_RECORDER

    /* get recorder status */
  case FMSTR_CMD_GETRECSTS:
    p_resp = FMSTR_GetRecStatus(msg);
    break;

    /* start recorder */
  case FMSTR_CMD_STARTREC:
    p_resp = FMSTR_StartRec(msg);
    break;

    /* stop recorder */
  case FMSTR_CMD_STOPREC:
    p_resp = FMSTR_StopRec(msg);
    break;

    /* setup recorder */
  case FMSTR_CMD_SETUPREC_EX:
    FMSTR_SetExAddr(FMSTR_TRUE);
    p_resp = FMSTR_SetUpRec(msg);
    break;

    /* get recorder buffer information (force EX instead of non-EX) */
  case FMSTR_CMD_GETRECBUFF_EX:
    FMSTR_SetExAddr(FMSTR_TRUE);
    p_resp = FMSTR_GetRecBuff(msg);
    break;
  #endif /* FMSTR_USE_RECORDER */

  #if FMSTR_USE_APPCMD

    /* accept the application command */
  case FMSTR_CMD_SENDAPPCMD:
    p_resp = FMSTR_StoreAppCmd(msg);
    break;

    /* get the application command status */
  case FMSTR_CMD_GETAPPCMDSTS:
    p_resp = FMSTR_GetAppCmdStatus(msg);
    break;

    /* get the application command data */
  case FMSTR_CMD_GETAPPCMDDATA:
    p_resp = FMSTR_GetAppCmdRespData(msg);
    break;
  #endif /* FMSTR_USE_APPCMD */

  #if FMSTR_USE_WRITEMEM

    /* write a block of memory */
  case FMSTR_CMD_WRITEMEM_EX:
    FMSTR_SetExAddr(FMSTR_TRUE);
    p_resp = FMSTR_WriteMem(msg);
    break;
  #endif /* FMSTR_USE_WRITEMEM */

  #if FMSTR_USE_WRITEMEMMASK

    /* write block of memory with a bit mask */
  case FMSTR_CMD_WRITEMEMMASK_EX:
    FMSTR_SetExAddr(FMSTR_TRUE);
    p_resp = FMSTR_WriteMemMask(msg);
    break;
  #endif /* FMSTR_USE_WRITEMEMMASK */

  #if FMSTR_USE_WRITEVAR && FMSTR_USE_NOEX_CMDS

    /* write byte */
  case FMSTR_CMD_WRITEVAR8:
    p_resp = FMSTR_WriteVar(msg, 1U);
    break;

    /* write word */
  case FMSTR_CMD_WRITEVAR16:
    p_resp = FMSTR_WriteVar(msg, 2U);
    break;

    /* write dword */
  case FMSTR_CMD_WRITEVAR32:
    p_resp = FMSTR_WriteVar(msg, 4U);
    break;
  #endif /* FMSTR_USE_WRITEVAR && FMSTR_USE_NOEX_CMDS */

  #if FMSTR_USE_WRITEVARMASK && FMSTR_USE_NOEX_CMDS

    /* write byte with mask */
  case FMSTR_CMD_WRITEVAR8MASK:
    p_resp = FMSTR_WriteVarMask(msg, 1U);
    break;

    /* write word with mask */
  case FMSTR_CMD_WRITEVAR16MASK:
    p_resp = FMSTR_WriteVarMask(msg, 2U);
    break;

  #endif /* FMSTR_USE_WRITEVARMASK && FMSTR_USE_NOEX_CMDS */

  #if FMSTR_USE_TSA

    /* get TSA table (force EX instead of non-EX) */
  case FMSTR_CMD_GETTSAINFO_EX:
    FMSTR_SetExAddr(FMSTR_TRUE);
    p_resp = FMSTR_GetTsaInfo(msg);
    break;

  case FMSTR_CMD_GETSTRLEN_EX:
    FMSTR_SetExAddr(FMSTR_TRUE);
    p_resp = FMSTR_GetStringLen(msg);
    break;

  #endif /* FMSTR_USE_TSA */

  #if FMSTR_USE_BRIEFINFO
    /* retrieve a subset of board information structure */
  case FMSTR_CMD_GETINFOBRIEF:
  #else
    /* retrieve board information structure */
  case FMSTR_CMD_GETINFO:
  #endif
    p_resp = FMSTR_GetBoardInfo(msg);
    break;

  #if FMSTR_USE_SFIO
  case FMSTR_CMD_SFIOFRAME_0:
  case FMSTR_CMD_SFIOFRAME_1:
    p_resp = FMSTR_SfioFrame(msg);
    break;
  case FMSTR_CMD_SFIOGETRESP_0:
  case FMSTR_CMD_SFIOGETRESP_1:
    p_resp = FMSTR_SfioGetResp(msg);
    break;
  #endif /* FMSTR_USE_SFIO */

  #if FMSTR_USE_PIPES
  case FMSTR_CMD_PIPE:
    p_resp = FMSTR_PipeFrame(msg);
    break;
  #endif /* FMSTR_USE_PIPES */

    /* unknown command */
  default:
    p_resp = FMSTR_ConstToBuffer8(msg, FMSTR_STC_INVCMD);
    break;
  }

  /* anything to send back? */
  if (p_resp != msg)
  {
    /*lint -e{946,960} subtracting pointers is appropriate here */
    uint8_t nSize = (uint8_t)(p_resp - msg);
  #if FMSTR_DEBUG_TX
    /* the first sane frame received from PC Host ends test frame sending */
    pcm_bDebugTx = 0;
  #endif
    FMSTR_SendResponse(msg, nSize);
    return FMSTR_TRUE;
  }
  else
  {
    /* nothing sent out */
    return FMSTR_FALSE;
  }
}

/**************************************************************************//*!
*
* @brief    Sending debug test frame
*
* @param    pMessageIO - outut frame buffer
*
* @return   True if successful (always in current implementation)
*
******************************************************************************/

int32_t FMSTR_SendTestFrame(uint8_t *pMessageIO)
{
  uint8_t *pResponse = pMessageIO;
  pResponse = FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_DEBUGTX_TEST);
  FMSTR_SendResponse(pMessageIO, 1);
  return FMSTR_TRUE;
}

/**************************************************************************//*!
*
* @brief    Handling GETINFO or GETINFO_BRIEF
*
* @param    pMessageIO - original command (in) and response buffer (out)
*
* @return   As all command handlers, the return value should be the buffer
*           pointer where the response output finished (except checksum)
*
******************************************************************************/

uint8_t*  FMSTR_GetBoardInfo(uint8_t *pMessageIO)
{
  uint8_t *pResponse = pMessageIO;
  uint16_t wTmp;
  uint8_t *pStr;

  pResponse = FMSTR_ConstToBuffer8(pResponse, FMSTR_STS_OK);
  pResponse = FMSTR_ConstToBuffer8(pResponse, (uint8_t)(FMSTR_PROT_VER));            /* protVer */
  pResponse = FMSTR_ConstToBuffer8(pResponse, (uint8_t)(FMSTR_CFG_FLAGS));           /* cfgFlags */
  pResponse = FMSTR_ConstToBuffer8(pResponse, (uint8_t)(FMSTR_CFG_BUS_WIDTH));       /* dataBusWdt */
  pResponse = FMSTR_ConstToBuffer8(pResponse, (uint8_t)(FMSTR_GLOB_VERSION_MAJOR));  /* globVerMajor */
  pResponse = FMSTR_ConstToBuffer8(pResponse, (uint8_t)(FMSTR_GLOB_VERSION_MINOR));  /* globVerMinor */
  pResponse = FMSTR_ConstToBuffer8(pResponse, (uint8_t)(FMSTR_COMM_BUFFER_SIZE));    /* cmdBuffSize  */

  /* that is all for brief info */
  #if FMSTR_USE_BRIEFINFO
  FMSTR_UNUSED(pStr);
  FMSTR_UNUSED(wTmp);

  #else /* FMSTR_USE_BRIEFINFO */

    #if FMSTR_USE_RECORDER

  /* recorder buffer size is always measured in bytes */
#if FMSTR_REC_LARGE_MODE
    wTmp = FMSTR_GetRecBuffSize()/64;
#else
    wTmp = FMSTR_GetRecBuffSize();
#endif
  wTmp =  wTmp * FMSTR_CFG_BUS_WIDTH;

  /* send size and timebase    */
  pResponse = FMSTR_ValueToBuffer16(pResponse, wTmp);
  pResponse = FMSTR_ConstToBuffer16(pResponse, (uint16_t)FMSTR_REC_TIMEBASE);
    #else /* FMSTR_USE_RECORDER */

  FMSTR_UNUSED(wTmp);

  /* recorder info zeroed */
  pResponse = FMSTR_ConstToBuffer16(pResponse, 0);
  pResponse = FMSTR_ConstToBuffer16(pResponse, 0);
    #endif /* FMSTR_USE_RECORDER */

    #if FMSTR_LIGHT_VERSION
  FMSTR_UNUSED(pStr);
  pResponse = FMSTR_SkipInBuffer(pResponse, (uint8_t)FMSTR_DESCR_SIZE);
    #else
  /* description string */
  pStr = (uint8_t *)FMSTR_IDT_STRING;
  for (wTmp = 0U; wTmp < (uint8_t)(FMSTR_DESCR_SIZE); wTmp++)
  {
    pResponse = FMSTR_ValueToBuffer8(pResponse, *pStr);

    /* terminating zero used to clear the remainder of the buffer */
    if (*pStr)
    {
      pStr++;
    }
  }
    #endif /* SEND_IDT_STRING */

  #endif /* FMSTR_USE_BRIEFINFO */

  return pResponse;
}

/**************************************************************************//*!
*
* @brief    Handling READMEM and READMEM_EX commands
*
* @param    pMessageIO - original command (in) and response buffer (out)
*
* @return   As all command handlers, the return value should be the buffer
*           pointer where the response output finished (except checksum)
*
******************************************************************************/

uint8_t*  FMSTR_ReadMem(uint8_t *pMessageIO)
{
  uint8_t *pResponse = pMessageIO;
  uint8_t *nAddr;
  uint8_t nSize;

  pMessageIO = FMSTR_SkipInBuffer(pMessageIO, 2U);
  pMessageIO = FMSTR_ValueFromBuffer8(&nSize, pMessageIO);
  pMessageIO = FMSTR_AddressFromBuffer(&nAddr, pMessageIO);

  #if FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY
  if (!FMSTR_CheckTsaSpace(nAddr, (uint8_t)nSize, FMSTR_FALSE))
  {
    return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_EACCESS);
  }
  #endif

  /* check the response will safely fit into comm buffer */
  if (nSize > (uint8_t)FMSTR_COMM_BUFFER_SIZE)
  {
    return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_RSPBUFFOVF);
  }

  /* success  */
  pResponse = FMSTR_ConstToBuffer8(pResponse, FMSTR_STS_OK);

  return FMSTR_CopyToBuffer(pResponse, nAddr, (uint8_t)nSize);
}

/**************************************************************************//*!
*
* @brief    Handling READVAR and READVAR_EX commands (for all sizes 1,2,4)
*
* @param    pMessageIO - original command (in) and response buffer (out)
*
* @return   As all command handlers, the return value should be the buffer
*           pointer where the response output finished (except checksum)
*
******************************************************************************/

uint8_t*  FMSTR_ReadVar(uint8_t *pMessageIO, uint8_t nSize)
{
  uint8_t *pResponse = pMessageIO;
  uint8_t *nAddr;

  pMessageIO = FMSTR_SkipInBuffer(pMessageIO, 1U);
  pMessageIO = FMSTR_AddressFromBuffer(&nAddr, pMessageIO);

  #if FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY
  if (!FMSTR_CheckTsaSpace(nAddr, nSize, FMSTR_FALSE))
  {
    return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_EACCESS);
  }
  #endif

  /* success  */
  pResponse = FMSTR_ConstToBuffer8(pResponse, FMSTR_STS_OK);

  return FMSTR_CopyToBuffer(pResponse, nAddr, nSize);
}

/**************************************************************************//*!
*
* @brief    Handling WRITEMEM and WRITEMEM_EX commands
*
* @param    pMessageIO - original command (in) and response buffer (out)
*
* @return   As all command handlers, the return value should be the buffer
*           pointer where the response output finished (except checksum)
*
******************************************************************************/

uint8_t*  FMSTR_WriteMem(uint8_t *pMessageIO)
{
  uint8_t *pResponse = pMessageIO;
  uint8_t *nAddr;
  uint8_t nSize,nResponseCode;

  pMessageIO = FMSTR_SkipInBuffer(pMessageIO, 2U);
  pMessageIO = FMSTR_ValueFromBuffer8(&nSize, pMessageIO);
  pMessageIO = FMSTR_AddressFromBuffer(&nAddr, pMessageIO);

  #if FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY
  if (!FMSTR_CheckTsaSpace(nAddr, (uint8_t)nSize, FMSTR_TRUE))
  {
    nResponseCode = FMSTR_STC_EACCESS;
    goto FMSTR_WriteMem_exit;
  }
  #endif

  /*lint -e{534} ignoring function return value */
  FMSTR_CopyFromBuffer(nAddr, pMessageIO, (uint8_t)nSize);
  nResponseCode = FMSTR_STS_OK;

  #if FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY
FMSTR_WriteMem_exit:
  #endif

  return FMSTR_ConstToBuffer8(pResponse, nResponseCode);
}

/**************************************************************************//*!
*
* @brief    Handling WRITEVAR command
*
* @param    pMessageIO - original command (in) and response buffer (out)
* @param    nSize - variable size
*
* @return   As all command handlers, the return value should be the buffer
*           pointer where the response output finished (except checksum)
*
******************************************************************************/

uint8_t*  FMSTR_WriteVar(uint8_t *pMessageIO, uint8_t nSize)
{
  uint8_t *pResponse = pMessageIO;
  uint8_t *nAddr;
  uint8_t nResponseCode;

  pMessageIO = FMSTR_SkipInBuffer(pMessageIO, 1U);
  pMessageIO = FMSTR_AddressFromBuffer(&nAddr, pMessageIO);

  #if FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY
  if (!FMSTR_CheckTsaSpace(nAddr, nSize, FMSTR_TRUE))
  {
    nResponseCode = FMSTR_STC_EACCESS;
    goto FMSTR_WriteVar_exit;
  }
  #endif

  /*lint -e{534} ignoring function return value */
  FMSTR_CopyFromBuffer(nAddr, pMessageIO, nSize);
  nResponseCode = FMSTR_STS_OK;

  #if FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY
FMSTR_WriteVar_exit:
  #endif

  return FMSTR_ConstToBuffer8(pResponse, nResponseCode);
}


/**************************************************************************//*!
*
* @brief    Handling WRITEMEMMASK and WRITEMEMMASK_EX commands
*
* @param    pMessageIO - original command (in) and response buffer (out)
*
* @return   As all command handlers, the return value should be the buffer
*           pointer where the response output finished (except checksum)
*
******************************************************************************/

uint8_t*  FMSTR_WriteMemMask(uint8_t *pMessageIO)
{
  uint8_t *pResponse = pMessageIO;
  uint8_t *nAddr;
  uint8_t nSize,nResponseCode;

  pMessageIO = FMSTR_SkipInBuffer(pMessageIO, 2U);
  pMessageIO = FMSTR_ValueFromBuffer8(&nSize, pMessageIO);
  pMessageIO = FMSTR_AddressFromBuffer(&nAddr, pMessageIO);

  #if FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY
  if (!FMSTR_CheckTsaSpace(nAddr, (uint8_t)nSize, FMSTR_TRUE))
  {
    nResponseCode = FMSTR_STC_EACCESS;
    goto FMSTR_WriteMemMask_exit;
  }
  #endif

  #if FMSTR_CFG_BUS_WIDTH > 1U
  /* size must be divisible by bus width (mask must not begin in half of memory word) */
  if (nSize % FMSTR_CFG_BUS_WIDTH)
  {
    nResponseCode = FMSTR_STC_INVSIZE;
    goto FMSTR_WriteMemMask_exit;
  }
  #endif

  /* put the data */
  FMSTR_CopyFromBufferWithMask(nAddr, pMessageIO, (uint8_t)nSize);
  nResponseCode = FMSTR_STS_OK;

  #if (FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY) || (FMSTR_CFG_BUS_WIDTH > 1U)
FMSTR_WriteMemMask_exit:
  #endif

  return FMSTR_ConstToBuffer8(pResponse, nResponseCode);
}

/**************************************************************************//*!
*
* @brief    Handling WRITEVARMASK command
*
* @param    pMessageIO - original command (in) and response buffer (out)
* @param    nSize - variable size
*
* @return   As all command handlers, the return value should be the buffer
*           pointer where the response output finished (except checksum)
*
******************************************************************************/

uint8_t*  FMSTR_WriteVarMask(uint8_t *pMessageIO, uint8_t nSize)
{
  uint8_t *pResponse = pMessageIO;
  uint8_t *nAddr;
  uint8_t nResponseCode;

  pMessageIO = FMSTR_SkipInBuffer(pMessageIO, 1U);
  pMessageIO = FMSTR_AddressFromBuffer(&nAddr, pMessageIO);

  #if FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY
  if (!FMSTR_CheckTsaSpace(nAddr, nSize, FMSTR_TRUE))
  {
    nResponseCode = FMSTR_STC_EACCESS;
    goto FMSTR_WriteVarMask_exit;
  }
  #endif

  /* put the data */
  FMSTR_CopyFromBufferWithMask(nAddr, pMessageIO, nSize);
  nResponseCode = FMSTR_STS_OK;

  #if FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY
FMSTR_WriteVarMask_exit:
  #endif

  return FMSTR_ConstToBuffer8(pResponse, nResponseCode);
}


/**************************************************************************//*!
*
* @brief    Private inline implementation of "strlen" used by TSA and Pipes
*
******************************************************************************/

uint16_t FMSTR_StrLen(uint8_t *nAddr)
{
  const char *pStr;
  uint16_t nLen = 0U;

  #ifdef __HCS12X__
  /* convert from logical to global if needed */
  nAddr = FMSTR_FixHcs12xAddr(nAddr);
  #endif

  /*lint -e{923} casting address value to pointer */
  pStr = (const char *)nAddr;

  while (*pStr++)
  {
    nLen++;
  }

  return nLen;
}
#else /* !FMSTR_DISABLE */

/**************************************************************************//*!
*
* @brief    FreeMASTER driver initialization is disabled
*
******************************************************************************/

int32_t FMSTR_Init(void)
{
  return FMSTR_FALSE;
}

/*******************************************************************************
*
* @brief    API: Main "Polling" call from the application main loop
*
*******************************************************************************/

void FMSTR_Poll(void)
{
}

/* HC12 interrupt routine declaration, must be in non-paged code memory */
  #if defined(FMSTR_PLATFORM_HC12) && (!defined(__S12Z__))
    #include "non_bank.sgm"
  #endif

/*******************************************************************************
*
* @brief    API: API: Main SCI / CAN Interrupt handler call
*
*******************************************************************************/
  #if !defined(FMSTR_PLATFORM_MQX)
    #if defined(FMSTR_PLATFORM_MPC55xx)
void FMSTR_Isr(unsigned long vec)
{
  FMSTR_UNUSED(vec);
}
    #else
void FMSTR_Isr(void)
{
}
    #endif
  #endif

  #if defined(FMSTR_PLATFORM_HC12) || defined(FMSTR_PLATFORM_HC08) || defined(FMSTR_PLATFORM_MCF51xx)
/*******************************************************************************
*
* @brief    API: API: The 2nd FMSTR interrupt handler
*
*******************************************************************************/

void FMSTR_Isr2(void)
{
}

  #endif

/* restore HC12 code segment */
  #if defined(FMSTR_PLATFORM_HC12) && (!defined(__S12Z__))
    #include "default.sgm"
  #endif

#endif /* !FMSTR_DISABLE */
