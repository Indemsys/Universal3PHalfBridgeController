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
* @brief  FreeMASTER Driver main API header file, MQX IO platform
*
* This is the only header file needed to be included by the user application
* to implement the FreeMASTER interface. In addition, user has to write the
* "freemaster_cfg.h" configuration file and put it anywhere on the #include path
*
*******************************************************************************/

#ifndef __FREEMASTER_H
  #define __FREEMASTER_H


#include "stdint.h"

/* identify our current platform */
  #define FMSTR_PLATFORM_MQX 1

/* FreeMASTER configuration */
  #include "freemaster_defcfg.h"



/* pointer to application command callback handler */
typedef uint8_t (*FMSTR_PAPPCMDFUNC)(uint8_t code, uint8_t *pdata, uint16_t size);


/* pointer to pipe event handler */
typedef void (*FMSTR_PPIPEFUNC)(void *);

/*****************************************************************************
* TSA-related user types and macros
******************************************************************************/

  #include "freemaster_tsa.h"

/*****************************************************************************
* Constants
******************************************************************************/

/* application command status information  */
  #define FMSTR_APPCMDRESULT_NOCMD      0xffU
  #define FMSTR_APPCMDRESULT_RUNNING    0xfeU
  #define MFSTR_APPCMDRESULT_LASTVALID  0xf7U  /* F8-FF are reserved  */

/* recorder time base declaration helpers */
  #define FMSTR_REC_BASE_SECONDS(x)  ((x) & 0x3fffU)
  #define FMSTR_REC_BASE_MILLISEC(x) (((x) & 0x3fffU) | 0x4000U)
  #define FMSTR_REC_BASE_MICROSEC(x) (((x) & 0x3fffU) | 0x8000U)
  #define FMSTR_REC_BASE_NANOSEC(x)  (((x) & 0x3fffU) | 0xc000U)

/******************************************************************************
* NULL needed
******************************************************************************/

  #ifndef NULL
    #ifdef __cplusplus
      #define NULL (0)
    #else
      #define NULL ((void *) 0)
    #endif
  #endif

/*****************************************************************************
* Global functions
******************************************************************************/
  #ifdef __cplusplus
extern "C"
{
  #endif

/* Assigning FreeMASTER communication module base address */
void     FMSTR_SetSciBaseAddress(uint8_t*  nSciAddr);
void     FMSTR_SetCanBaseAddress(uint8_t*  nCanAddr);

/* FreeMASTER serial communication API */
int32_t  FMSTR_Init(void);    /* general initialization */
void     FMSTR_Poll(void);          /* polling call, use in SHORT_INTR and POLL_DRIVEN modes */

/* Recorder API */
void     FMSTR_Recorder(void);
void     FMSTR_TriggerRec(void);
void     FMSTR_SetUpRecBuff(uint8_t*  nBuffAddr, uint32_t nBuffSize);

/* Application commands API */
uint8_t  FMSTR_GetAppCmd(void);
uint8_t* FMSTR_GetAppCmdData(uint16_t *pDataLen);
int32_t  FMSTR_RegisterAppCmdCall(uint8_t nAppCmdCode, FMSTR_PAPPCMDFUNC pCallbackFunc);

void     FMSTR_AppCmdAck(uint8_t nResultCode);
void     FMSTR_AppCmdSetResponseData(uint8_t*  nResultDataAddr, uint16_t nResultDataLen);

/* Dynamic TSA API */
int32_t  FMSTR_SetUpTsaBuff(uint8_t*  nBuffAddr, uint16_t nBuffSize);
int32_t  FMSTR_TsaAddVar(FMSTR_TSATBL_STRPTR pszName, FMSTR_TSATBL_STRPTR pszType,FMSTR_TSATBL_VOIDPTR nAddr, uint32_t nSize, uint16_t nFlags);

/* FreeMASTER LIN Transport Layer processing. User should call ld_receive_message() on his own. */
int32_t  FMSTR_IsLinTLFrame(uint8_t*  nFrameAddr, uint16_t nFrameSize);
int32_t  FMSTR_ProcessLinTLFrame(uint8_t*  nFrameAddr, uint16_t nFrameSize);

/* Loss-less Communication Pipes API */
void*    FMSTR_PipeOpen(uint8_t nPort, FMSTR_PPIPEFUNC pCallback,uint8_t*  pRxBuff, uint16_t nRxSize,uint8_t*  pTxBuff, uint16_t nTxSize);
void     FMSTR_PipeClose(void *hpipe);
uint16_t FMSTR_PipeWrite(void *hpipe, uint8_t*  addr, uint16_t length, uint16_t granularity);
uint16_t FMSTR_PipeRead(void *hpipe, uint8_t*  addr, uint16_t length, uint16_t granularity);

/* Pipe printing and formatting */
int32_t  FMSTR_PipePuts(void *hpipe, const char *pszStr);
int32_t  FMSTR_PipePrintf(void *hpipe, const char *pszFmt, ...);
int32_t  FMSTR_PipePrintfU8(void *hpipe, const char *pszFmt, unsigned char arg);
int32_t  FMSTR_PipePrintfS8(void *hpipe, const char *pszFmt, signed char arg);
int32_t  FMSTR_PipePrintfU16(void *hpipe, const char *pszFmt, unsigned short arg);
int32_t  FMSTR_PipePrintfS16(void *hpipe, const char *pszFmt, signed short arg);
int32_t  FMSTR_PipePrintfU32(void *hpipe, const char *pszFmt, uint32_t arg);
int32_t  FMSTR_PipePrintfS32(void *hpipe, const char *pszFmt, int32_t arg);

  #ifdef __cplusplus
}
  #endif

#endif /* __FREEMASTER_H */

