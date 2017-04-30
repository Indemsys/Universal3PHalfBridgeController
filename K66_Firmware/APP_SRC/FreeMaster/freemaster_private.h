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
* @brief  FreeMASTER driver private declarations, used internally by the driver
*
*******************************************************************************/

#ifndef __FREEMASTER_PRIVATE_H
  #define __FREEMASTER_PRIVATE_H

  //#include "bsp.h"


  #ifndef __FREEMASTER_H
    #error Please include the freemaster.h master header file before the freemaster_private.h
  #endif


/******************************************************************************
* Platform-dependent types, macros and functions
******************************************************************************/

  #undef FMSTR_PLATFORM


  #include "freemaster_os.h"


/******************************************************************************
* Boolean values
******************************************************************************/

  #ifndef FMSTR_TRUE
    #define FMSTR_TRUE (1U)
  #endif

  #ifndef FMSTR_FALSE
    #define FMSTR_FALSE (0U)
  #endif


/******************************************************************************
* inline functions
******************************************************************************/

/* we do not assume the inline is always supported by compiler
  rather each platform header defines its FMSTR_INLINE */
  #ifndef FMSTR_INLINE
    #define FMSTR_INLINE static
  #endif

/* building macro-based inline code */
  #define FMSTR_MACROCODE_BEGIN()     do{
  #define FMSTR_MACROCODE_END()       }while(0)

/******************************************************************************
* Global non-API functions (used internally in FreeMASTER driver)
******************************************************************************/

int32_t   FMSTR_InitSerial(void);
void      FMSTR_SendResponse(uint8_t *pMessageIO, uint8_t nLength);
void      FMSTR_ProcessSCI(void);
void      FMSTR_ProcessJTAG(void);
void      FMSTR_ProcessMBED(void);

int32_t   FMSTR_Tx(uint8_t *pTxChar);
int32_t   FMSTR_Rx(uint8_t   nRxChar);

int32_t   FMSTR_ProtocolDecoder(uint8_t *pMessageIO);
int32_t   FMSTR_SendTestFrame(uint8_t *pMessageIO);
uint8_t*  FMSTR_GetBoardInfo(uint8_t *pMessageIO);

uint8_t*  FMSTR_ReadMem(uint8_t *pMessageIO);
uint8_t*  FMSTR_ReadVar(uint8_t *pMessageIO, uint8_t nSize);
uint8_t*  FMSTR_WriteMem(uint8_t *pMessageIO);
uint8_t*  FMSTR_WriteVar(uint8_t *pMessageIO, uint8_t nSize);
uint8_t*  FMSTR_WriteVarMask(uint8_t *pMessageIO, uint8_t nSize);
uint8_t*  FMSTR_WriteMemMask(uint8_t *pMessageIO);

void      FMSTR_InitAppCmds(void);
uint8_t*  FMSTR_StoreAppCmd(uint8_t *pMessageIO);
uint8_t*  FMSTR_GetAppCmdStatus(uint8_t *pMessageIO);
uint8_t*  FMSTR_GetAppCmdRespData(uint8_t *pMessageIO);

void      FMSTR_InitScope(void);
uint8_t*  FMSTR_SetUpScope(uint8_t *pMessageIO);
uint8_t*  FMSTR_ReadScope(uint8_t *pMessageIO);

void      FMSTR_InitRec(void);
uint8_t*  FMSTR_SetUpRec(uint8_t *pMessageIO);
uint8_t*  FMSTR_StartRec(uint8_t *pMessageIO);
uint8_t*  FMSTR_StopRec(uint8_t *pMessageIO);
uint8_t*  FMSTR_GetRecStatus(uint8_t *pMessageIO);
uint8_t*  FMSTR_GetRecBuff(uint8_t *pMessageIO);
int32_t   FMSTR_IsInRecBuffer(uint8_t *nAddr, uint8_t nSize);
uint32_t  FMSTR_GetRecBuffSize(void);

void      FMSTR_InitTsa(void);
uint8_t*  FMSTR_GetTsaInfo(uint8_t *pMessageIO);
uint8_t*  FMSTR_GetStringLen(uint8_t *pMessageIO);
int32_t   FMSTR_CheckTsaSpace(uint8_t *nAddr, uint8_t nSize, int32_t bWriteAccess);
uint16_t  FMSTR_StrLen(uint8_t *nAddr);

void      FMSTR_InitSfio(void);
uint8_t*  FMSTR_SfioFrame(uint8_t *pMessageIO);
uint8_t*  FMSTR_SfioGetResp(uint8_t *pMessageIO);

void      FMSTR_InitPipes(void);
uint8_t*  FMSTR_PipeFrame(uint8_t *pMessageIO);

int32_t   FMSTR_InitCan(void);
void      FMSTR_SetCanCmdID(uint32_t canID);
void      FMSTR_SetCanRespID(uint32_t canID);
int32_t   FMSTR_TxCan(void);
int32_t   FMSTR_RxCan(void);
void      FMSTR_ProcessCanRx(void);
void      FMSTR_ProcessCanTx(void);
void      FMSTR_InitPDBdm(void);
int32_t   FMSTR_InitLinTL(void);

/****************************************************************************************
* Potentially unused variable declaration
*****************************************************************************************/
  #if defined(_lint) || defined(__IAR_SYSTEMS_ICC__) || defined(__ARMCC_VERSION)
    #define FMSTR_UNUSED(sym) /*lint -esym(715,sym) -esym(818,sym) -esym(529,sym) -e{960} */
  #elif defined(__GNUC__)
    #define FMSTR_UNUSED(x) (void)(x)
  #else
    #define FMSTR_UNUSED(sym) ((sym),0)
  #endif


  #if (FMSTR_LONG_INTR && (FMSTR_SHORT_INTR || FMSTR_POLL_DRIVEN)) || \
    (FMSTR_SHORT_INTR && (FMSTR_LONG_INTR || FMSTR_POLL_DRIVEN)) || \
    (FMSTR_POLL_DRIVEN && (FMSTR_LONG_INTR || FMSTR_SHORT_INTR)) || \
    !(FMSTR_POLL_DRIVEN || FMSTR_LONG_INTR || FMSTR_SHORT_INTR)
/* mismatch in interrupt modes, only one can be selected */
    #error You have to enable exctly one of FMSTR_LONG_INTR or FMSTR_SHORT_INTR or FMSTR_POLL_DRIVEN
  #endif

  #if FMSTR_SHORT_INTR
    #if FMSTR_COMM_RQUEUE_SIZE < 1
      #error Error in FMSTR_COMM_RQUEUE_SIZE value.
    #endif
  #endif

/* count how many interfaces are enabled (only one should be enabled) */
  #define FMSTR_COUNT_INTERFACES \
    ( ((FMSTR_DISABLE)?1:0) + ((FMSTR_USE_SCI)?1:0) + ((FMSTR_USE_ESCI)?1:0) + ((FMSTR_USE_LPUART)?1:0) + \
      ((FMSTR_USE_JTAG)?1:0) + ((FMSTR_USE_CAN)?1:0) + ((FMSTR_USE_LINTL)?1:0) + \
      ((FMSTR_USE_USB_CDC)?1:0) + ((FMSTR_USE_MBED)?1:0) )

/* only one communication link may be selected */
  #if (!(FMSTR_DISABLE)) && FMSTR_COUNT_INTERFACES > 1
    #error More than one communication interface selected for FreeMASTER driver.
  #endif

/* All Interface options are set to 0 */
  #if FMSTR_COUNT_INTERFACES == 0
    #error No FreeMASTER communication interface is enabled. Please choose one intercace or set FMSTR_DISABLE option to 1.
  #endif


/* MQX doesn't supports Long interrupt and Short interrupt */
    #if FMSTR_LONG_INTR
      #warning "MQX IO interface doesn't supports the Long Interrupt routines"
    #elif FMSTR_SHORT_INTR
      #warning "MQX IO interface doesn't supports the normal Short Interrupt routines, please open your MQX communication interface in Interrupt mode"
    #endif
  #endif


  #if (FMSTR_USE_SCI) || (FMSTR_USE_ESCI) || (FMSTR_USE_LPUART) || (FMSTR_USE_JTAG) || (FMSTR_USE_USB_CDC) || (FMSTR_USE_MBED)
    #ifndef FMSTR_USE_SERIAL
      #define FMSTR_USE_SERIAL 1
    #else
      #if FMSTR_USE_SERIAL == 0
        #error "FMSTR_USE_SERIAL macro cannot be configured by user, FreeMASTER serial driver functionality is not garanted."
      #endif
    #endif
  #else
    #ifndef FMSTR_USE_SERIAL
      #define FMSTR_USE_SERIAL 0
    #endif
  #endif



/* check scope settings */
  #if FMSTR_USE_SCOPE
    #if FMSTR_MAX_SCOPE_VARS > 32 || FMSTR_MAX_SCOPE_VARS < 2
      #error Error in FMSTR_MAX_SCOPE_VARS value. Use a value in range 2..32
    #endif
  #endif

/* check recorder settings */
  #if (FMSTR_USE_RECORDER) || (FMSTR_USE_FASTREC)
    #if FMSTR_MAX_REC_VARS > 32 || FMSTR_MAX_REC_VARS < 2
      #error Error in FMSTR_MAX_REC_VARS value. Use a value in range 2..32
    #endif

    #if !FMSTR_USE_READMEM
      #error Recorder needs the FMSTR_USE_READMEM feature
    #endif
  #endif

/* fast recorder requires its own allocation of recorder buffer */
  #if FMSTR_USE_FASTREC
    #if FMSTR_REC_OWNBUFF
      #error Fast recorder requires its own buffer allocation
    #endif
  #endif

  #if FMSTR_USE_TSA
    #if !FMSTR_USE_READMEM
      #error TSA needs the FMSTR_USE_READMEM feature
    #endif
  #endif

/* automatic buffer size by default */
  #ifndef FMSTR_COMM_BUFFER_SIZE
    #define FMSTR_COMM_BUFFER_SIZE 0
  #endif

/* check minimal buffer size required for all enabled features */
  #if FMSTR_COMM_BUFFER_SIZE

/* basic commands (getinfobrief, write/read memory etc.) */
    #if FMSTR_USE_BRIEFINFO && (FMSTR_COMM_BUFFER_SIZE < 11)
      #error FMSTR_COMM_BUFFER_SIZE set too small for basic FreeMASTER commands (11 bytes)
    #endif

/* full info required */
    #if !(FMSTR_USE_BRIEFINFO) && (FMSTR_COMM_BUFFER_SIZE < (35+1))
      #error FMSTR_COMM_BUFFER_SIZE set too small for GETINFO command (size 35+1)
    #endif

/* application commands */
    #if FMSTR_USE_APPCMD && (FMSTR_COMM_BUFFER_SIZE < ((FMSTR_APPCMD_BUFF_SIZE)+1+2))
      #error FMSTR_COMM_BUFFER_SIZE set too small for SENDAPPCMD command (size FMSTR_APPCMD_BUFF_SIZE+1+2)
    #endif

/* configuring scope (EX) */
    #if FMSTR_USE_SCOPE && (FMSTR_COMM_BUFFER_SIZE < (((FMSTR_MAX_SCOPE_VARS)*5)+1+2))
      #error FMSTR_COMM_BUFFER_SIZE set too small for SETUPSCOPEEX command (size FMSTR_MAX_SCOPE_VARS*5+1+2)
    #endif

/* configuring recorder (EX) */
    #if FMSTR_USE_RECORDER && (FMSTR_COMM_BUFFER_SIZE < (((FMSTR_MAX_REC_VARS)*5)+18+2))
      #error FMSTR_COMM_BUFFER_SIZE set too small for SETUPRECEX command (size FMSTR_MAX_REC_VARS*5+18+2)
    #endif



#endif /* __FREEMASTER_PRIVATE_H */

