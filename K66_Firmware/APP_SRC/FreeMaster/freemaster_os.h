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
* @brief  FreeMASTER Driver OS hardware dependent stuff
*
*******************************************************************************/

#ifndef __FREEMASTER_MQX_H
  #define __FREEMASTER_MQX_H


/*****************************************************************************
* Board configuration information
******************************************************************************/

  #define FMSTR_PROT_VER           3      /* protocol version 3 */
  #define FMSTR_CFG_FLAGS          FMSTR_CFG_REC_LARGE_MODE      /* board info flags */
  #define FMSTR_CFG_BUS_WIDTH      1      /* data bus width */
  #define FMSTR_GLOB_VERSION_MAJOR 2      /* driver version */
  #define FMSTR_GLOB_VERSION_MINOR 0
  #define FMSTR_IDT_STRING "FreeMASTER Drv."
  #define FMSTR_TSA_FLAGS          0



void      FMSTR_CopyMemory(uint8_t *nDestAddr, uint8_t *nSrcAddr, uint8_t nSize);
uint8_t*  FMSTR_CopyToBuffer(uint8_t *pDestBuff, uint8_t *nSrcAddr, uint8_t nSize);
uint8_t*  FMSTR_CopyFromBuffer(uint8_t *nDestAddr, uint8_t *pSrcBuff, uint8_t nSize);
void      FMSTR_CopyFromBufferWithMask(uint8_t *nDestAddr, uint8_t *pSrcBuff, uint8_t nSize);

#define   FMSTR_SetExAddr(X)

/*********************************************************************************
* communication buffer access functions. Most of them are trivial simple on MQX
*********************************************************************************/

  #define FMSTR_ValueFromBuffer8(pDest, pSrc)    ( (*((uint8_t*) (pDest)) = *(uint8_t*) (pSrc)), (((uint8_t* )(pSrc))+1) )
  #define FMSTR_ValueFromBuffer16(pDest, pSrc)   ( (*((uint16_t*)(pDest)) = *(uint16_t*)(pSrc)), (((uint8_t* )(pSrc))+2) )
  #define FMSTR_ValueFromBuffer32(pDest, pSrc)   ( (*((uint32_t*)(pDest)) = *(uint32_t*)(pSrc)), (((uint8_t* )(pSrc))+4) )
  #define FMSTR_ValueToBuffer8(pDest, src)       ( (*((uint8_t*) (pDest)) =  (uint8_t)  (src)) , (((uint8_t* )(pDest))+1) )
  #define FMSTR_ValueToBuffer16(pDest, src)      ( (*((uint16_t*)(pDest)) =  (uint16_t) (src)) , (((uint8_t* )(pDest))+2) )
  #define FMSTR_ValueToBuffer32(pDest, src)      ( (*((uint32_t*)(pDest)) =  (uint32_t) (src)) , (((uint8_t* )(pDest))+4) )


  #define FMSTR_SkipInBuffer(pDest, nSize)       ( ((uint8_t* )(pDest)) + (nSize) )


  #define FMSTR_ConstToBuffer8  FMSTR_ValueToBuffer8
  #define FMSTR_ConstToBuffer16 FMSTR_ValueToBuffer16

  #define FMSTR_AddressFromBuffer(pDest, pSrc) FMSTR_ValueFromBuffer32(pDest, pSrc)
  #define FMSTR_AddressToBuffer(pDest, nAddr)  FMSTR_ValueToBuffer32(pDest, nAddr)


  #define FMSTR_GetS8(addr)  ( *(int8_t*)(addr) )
  #define FMSTR_GetU8(addr)  ( *(uint8_t*)(addr) )
  #define FMSTR_GetS16(addr) ( *(int16_t*)(addr) )
  #define FMSTR_GetU16(addr) ( *(uint16_t*)(addr) )
  #define FMSTR_GetS32(addr) ( *(int32_t*)(addr) )
  #define FMSTR_GetU32(addr) ( *(uint32_t*)(addr) )

  #if FMSTR_REC_FLOAT_TRIG
    #define FMSTR_GetFloat(addr) ( *(float*)(addr) )
  #endif

/****************************************************************************************
* Other helper macros
*****************************************************************************************/

/* This macro assigns C pointer to FMSTR_ADDR-typed variable */
  #define FMSTR_PTR2ADDR(tmpAddr,ptr) ( tmpAddr = (uint8_t* ) (uint8_t*) ptr )
  #define FMSTR_ARR2ADDR FMSTR_PTR2ADDR

#endif /* __FREEMASTER_MQX_H */
