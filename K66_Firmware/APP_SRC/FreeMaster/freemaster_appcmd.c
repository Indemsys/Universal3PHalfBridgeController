/*******************************************************************************
*
* Copyright 2004-2013 Freescale Semiconductor, Inc.
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale FreeMASTER License
* distributed with this Material.
* See the LICENSE file distributed for more details.
*
****************************************************************************//*!
*
* @brief  FreeMASTER Application Commands implementation
*
*******************************************************************************/

#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_protocol.h"

#if (FMSTR_USE_APPCMD) && (!(FMSTR_DISABLE))

/***********************************
*  local variables
***********************************/

static uint8_t pcm_nAppCmd;                               /* app.cmd code (to application) */
static uint8_t pcm_pAppCmdBuff[FMSTR_APPCMD_BUFF_SIZE];   /* app.cmd data buffer  */
static uint16_t        pcm_nAppCmdLen;                            /* app.cmd data length */

static uint8_t  pcm_nAppCmdResult;                      /* app.cmd result code (from application) */
static uint8_t pcm_nAppCmdResultDataLen;

#if FMSTR_MAX_APPCMD_CALLS > 0
static uint8_t pcm_pAppCmdCallId[FMSTR_MAX_APPCMD_CALLS]; /* registered callback commands */
static FMSTR_PAPPCMDFUNC pcm_pAppCmdCallFunc[FMSTR_MAX_APPCMD_CALLS];  /* registered callback handlers */
#endif

/***********************************
*  local functions
***********************************/

static int16_t FMSTR_FindAppCmdCallIndex(uint8_t nAppcmdCode);

/**************************************************************************//*!
*
* @brief    Initialize app.cmds interface
*
******************************************************************************/

void FMSTR_InitAppCmds(void)
{
#if FMSTR_MAX_APPCMD_CALLS
    int16_t i;

    for(i=0; i<FMSTR_MAX_APPCMD_CALLS; i++)
    {
        pcm_pAppCmdCallId[i] = FMSTR_APPCMDRESULT_NOCMD;
        pcm_pAppCmdCallFunc[i] = NULL;
    }
#endif

    pcm_nAppCmd = (uint8_t) FMSTR_APPCMDRESULT_NOCMD;
    pcm_nAppCmdResult = (uint8_t) FMSTR_APPCMDRESULT_NOCMD;
}

/**************************************************************************//*!
*
* @brief    Handling SANDAPPCMD command
*
* @param    pMessageIO - original command (in) and response buffer (out)
*
* @return   As all command handlers, the return value should be the buffer
*           pointer where the response output finished (except checksum)
*
******************************************************************************/

uint8_t*  FMSTR_StoreAppCmd(uint8_t*  pMessageIO)
{
    uint8_t*  pResponse = pMessageIO;
    uint8_t nArgsLen;
    uint8_t nCode;

    /* the previous command not yet processed */
    if(pcm_nAppCmd != ((uint8_t) FMSTR_APPCMDRESULT_NOCMD))
    {
        return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_SERVBUSY);
    }

    pMessageIO = FMSTR_SkipInBuffer(pMessageIO, 1U);
    pMessageIO = FMSTR_ValueFromBuffer8(&nArgsLen, pMessageIO);
    pMessageIO = FMSTR_ValueFromBuffer8(&nCode, pMessageIO);

    /* args len is datalen minus one */
    nArgsLen--;

    /* does the application command fit to buffer ? */
    if (nArgsLen > (uint8_t) FMSTR_APPCMD_BUFF_SIZE)
    {
        return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_INVBUFF);
    }

    /* store command data into dedicated buffer */
    pcm_nAppCmd = nCode;
    pcm_nAppCmdLen = nArgsLen;

    /* data copy */
    if(nArgsLen)
    {
        uint8_t*  appCmdBuffAddr;
        FMSTR_ARR2ADDR(appCmdBuffAddr, pcm_pAppCmdBuff);

        /*lint -e{534} ignoring return value */
        FMSTR_CopyFromBuffer(appCmdBuffAddr, pMessageIO, (uint8_t) nArgsLen);
    }

    /* mark command as "running" (without any response data) */
    pcm_nAppCmdResult = (uint8_t) FMSTR_APPCMDRESULT_RUNNING;
    pcm_nAppCmdResultDataLen = 0U;

    return FMSTR_ConstToBuffer8(pResponse, FMSTR_STS_OK);
}

/**************************************************************************//*!
*
* @brief    Handling GETAPPCMDSTS command
*
* @param    pMessageIO - original command (in) and response buffer (out)
*
* @return   As all command handlers, the return value should be the buffer
*           pointer where the response output finished (except checksum)
*
* @note The callback-registered commands are processed at the moment the PC
*       tries to get the result for the first time. At this moment, we are
*       sure the PC already got the command delivery packet acknowledged.
*
******************************************************************************/

uint8_t*  FMSTR_GetAppCmdStatus(uint8_t*  pMessageIO)
{
#if FMSTR_MAX_APPCMD_CALLS
    FMSTR_PAPPCMDFUNC pFunc = NULL;
    int16_t nIndex;

    /* time to execute the command's callback */
    if((nIndex = FMSTR_FindAppCmdCallIndex(pcm_nAppCmd)) >= 0)
    {
        pFunc = pcm_pAppCmdCallFunc[nIndex];
    }

    /* valid callback function found? */
    if(pFunc)
    {
        /* do execute callback, return value is app.cmd result code */
        pcm_nAppCmdResult = pFunc(pcm_nAppCmd, pcm_pAppCmdBuff, pcm_nAppCmdLen);

        /* nothing more to do with this command (i.e. command acknowledged) */
        pcm_nAppCmd = FMSTR_APPCMDRESULT_NOCMD;
    }
#endif

    pMessageIO = FMSTR_ConstToBuffer8(pMessageIO, FMSTR_STS_OK);
    return FMSTR_ValueToBuffer8(pMessageIO, (uint8_t) pcm_nAppCmdResult);
}

/**************************************************************************//*!
*
* @brief    Handling GETAPPCMDDATA command
*
* @param    pMessageIO - original command (in) and response buffer (out)
*
* @return   As all command handlers, the return value should be the buffer
*           pointer where the response output finished (except checksum)
*
******************************************************************************/

uint8_t*  FMSTR_GetAppCmdRespData(uint8_t*  pMessageIO)
{
    uint8_t*  pResponse = pMessageIO;
    uint8_t nDataLen;
    uint8_t nDataOffset;

    /* the previous command not yet processed */
    if(pcm_nAppCmd != ((uint8_t)FMSTR_APPCMDRESULT_NOCMD))
    {
        return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_SERVBUSY);
    }

    pMessageIO = FMSTR_SkipInBuffer(pMessageIO, 1U);
    pMessageIO = FMSTR_ValueFromBuffer8(&nDataLen, pMessageIO);
    pMessageIO = FMSTR_ValueFromBuffer8(&nDataOffset, pMessageIO);

    /* the response would not fit into comm buffer */
    if(nDataLen > (uint16_t)FMSTR_COMM_BUFFER_SIZE)
    {
        return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_RSPBUFFOVF);
    }

    /* the data would be fetched outside the app.cmd response data */
    if((((uint16_t)nDataOffset) + nDataLen) > (uint8_t)pcm_nAppCmdResultDataLen)
    {
        return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_INVSIZE);
    }

    pResponse = FMSTR_ConstToBuffer8(pResponse, FMSTR_STS_OK);

    /* copy to buffer */
    {
        uint8_t*  appCmdBuffAddr;
        FMSTR_ARR2ADDR(appCmdBuffAddr, pcm_pAppCmdBuff);
        pResponse = FMSTR_CopyToBuffer(pResponse, appCmdBuffAddr, (uint8_t)nDataLen);
    }

    return pResponse;
}

/**************************************************************************//*!
*
* @brief    Find index of registered app.cmd callback
*
* @param    nAppcmdCode - App. command ID
*
* @return   Index of function pointer in our local tables
*
******************************************************************************/

static int16_t FMSTR_FindAppCmdCallIndex(uint8_t nAppcmdCode)
{
#if FMSTR_MAX_APPCMD_CALLS > 0
    int16_t i;

    for(i=0; i<FMSTR_MAX_APPCMD_CALLS; i++)
    {
        if(pcm_pAppCmdCallId[i] == nAppcmdCode)
        {
            return i;
        }
    }
#else
    /*lint -esym(528, FMSTR_FindAppCmdCallIndex) this function is
      not referenced when APPCMD_CALLS are not used */
    FMSTR_UNUSED(nAppcmdCode);
#endif

    return -1;
}

/**************************************************************************//*!
*
* @brief    API: Mark the application command is processed by the application
*
* @param    nResultCode - the result code which is returned to a host
*
******************************************************************************/

void FMSTR_AppCmdAck(uint8_t nResultCode)
{
    pcm_nAppCmdResult = nResultCode;
    pcm_nAppCmdResultDataLen = 0U;

    /* waiting for a new command to come */
    pcm_nAppCmd = (uint8_t)FMSTR_APPCMDRESULT_NOCMD;
}

/**************************************************************************//*!
*
* @brief    API: Mark the application command is processed by the application
*
* @param    pResultDataAddr - address of data we want to return to the PC
* @param    nResultDataLen - length of return data
*
******************************************************************************/

void FMSTR_AppCmdSetResponseData(uint8_t*  nResultDataAddr, uint16_t nResultDataLen)
{
    /* any data supplied by user? */
    if(nResultDataAddr)
    {
        /* response data length is trimmed if response data would not fit into buffer */
        pcm_nAppCmdResultDataLen = (uint8_t) nResultDataLen;
        if(pcm_nAppCmdResultDataLen > (uint8_t) FMSTR_APPCMD_BUFF_SIZE)
        {
            pcm_nAppCmdResultDataLen = (uint8_t) FMSTR_APPCMD_BUFF_SIZE;
        }

        if(pcm_nAppCmdResultDataLen > 0U)
        {
            uint8_t*  appCmdBuffAddr;
            FMSTR_ARR2ADDR(appCmdBuffAddr, pcm_pAppCmdBuff);
            FMSTR_CopyMemory(appCmdBuffAddr, nResultDataAddr, pcm_nAppCmdResultDataLen);
        }
    }
    else
    {
        /* no data being returned at all (same effect as pure FMSTR_AppCmdAck) */
        pcm_nAppCmdResultDataLen = 0U;
    }
}

/**************************************************************************//*!
*
* @brief    API: Fetch the application command code if one is ready for processing
*
* @return   A command code stored in the application cmd buffer.
*           The return value is FMSTR_APPCMDRESULT_NOCMD if there is no
*           new command since the last call to FMSTR_AppCmdAck
*
******************************************************************************/

uint8_t FMSTR_GetAppCmd(void)
{
#if FMSTR_MAX_APPCMD_CALLS
    /* the user can never see the callback-registered commands */
    if(FMSTR_FindAppCmdCallIndex(pcm_nAppCmd) >= 0)
    {
        return FMSTR_APPCMDRESULT_NOCMD;
    }
#endif

    /* otherwise, return the appcomand pending */
    return pcm_nAppCmd;
}

/**************************************************************************//*!
*
* @brief    API: Get a pointer to application command data
*
* @param    pDataLen - A pointer to variable which receives the data length
*
* @return   Pointer to the "application command" data
*
******************************************************************************/

uint8_t*  FMSTR_GetAppCmdData(uint16_t* pDataLen)
{
    /* no command, no data */
    if(pcm_nAppCmd == ((uint8_t)FMSTR_APPCMDRESULT_NOCMD))
    {
        return NULL;
    }

#if FMSTR_MAX_APPCMD_CALLS
    /* the user never sees the callback-registered commands */
    if(FMSTR_FindAppCmdCallIndex(pcm_nAppCmd) >= 0)
    {
        return NULL;
    }
#endif

    /* caller want to know the data length */
    if(pDataLen)
    {
        *pDataLen = pcm_nAppCmdLen;
    }

    /* data are saved in out buffer */
    return pcm_nAppCmdLen ? pcm_pAppCmdBuff : (uint8_t*) NULL;
}

/**************************************************************************//*!
*
* @brief    API: Register or unregister app.cmd callback handler
*
* @param    nAppCmdCode   - App.command ID
* @param    pCallbackFunc - Pointer to handler function (NULL to unregister)
*
* @return   Non-zero if successfull, zero if maximum callbacks already set
*
******************************************************************************/

int32_t FMSTR_RegisterAppCmdCall(uint8_t nAppCmdCode, FMSTR_PAPPCMDFUNC pCallbackFunc)
{
#if FMSTR_MAX_APPCMD_CALLS > 0

    int16_t nIndex;

    /* keep "void" ID as reserved */
    if(nAppCmdCode == FMSTR_APPCMDRESULT_NOCMD)
    {
        return FMSTR_FALSE;
    }

    /* get index of app.cmd ID (if already set) */
    nIndex = FMSTR_FindAppCmdCallIndex(nAppCmdCode);

    /* when not found, get a free slot (only if registering new callback) */
    if((nIndex < 0) && (pCallbackFunc != NULL))
    {
        nIndex = FMSTR_FindAppCmdCallIndex(FMSTR_APPCMDRESULT_NOCMD);
    }

    /* failed? */
    if(nIndex < 0)
    {
        return FMSTR_FALSE;
    }

    /* register handler */
    pcm_pAppCmdCallFunc[nIndex] = pCallbackFunc;
    pcm_pAppCmdCallId[nIndex] = (uint8_t) (pCallbackFunc ?
        nAppCmdCode : FMSTR_APPCMDRESULT_NOCMD);

    return FMSTR_TRUE;

#else
    FMSTR_UNUSED(pCallbackFunc);
    FMSTR_UNUSED(nAppCmdCode);

    /* app.cmd callback not implemented */
    return FMSTR_FALSE;

#endif
}

#else /* FMSTR_USE_APPCMD && (!FMSTR_DISABLE) */

/* void Application Command API functions */

void FMSTR_AppCmdAck(uint8_t nResultCode)
{
    FMSTR_UNUSED(nResultCode);
}

void FMSTR_AppCmdSetResponseData(uint8_t*  pResultData, uint16_t nResultDataLen)
{
    FMSTR_UNUSED(pResultData);
    FMSTR_UNUSED(nResultDataLen);
}

uint8_t FMSTR_GetAppCmd(void)
{
    return (uint8_t) FMSTR_APPCMDRESULT_NOCMD;
}

uint8_t* FMSTR_GetAppCmdData(uint16_t* pDataLen)
{
    FMSTR_UNUSED(pDataLen);
    return NULL;
}

int32_t FMSTR_RegisterAppCmdCall(uint8_t nAppCmdCode, FMSTR_PAPPCMDFUNC pCallbackFunc)
{
    FMSTR_UNUSED(nAppCmdCode);
    FMSTR_UNUSED(pCallbackFunc);
    return FMSTR_FALSE;
}

/*lint -efile(766, freemaster_protocol.h) include file is not used in this case */

#endif /* FMSTR_USE_APPCMD && (!FMSTR_DISABLE) */
