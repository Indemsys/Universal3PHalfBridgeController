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
* @brief  FreeMASTER Driver MQX dependent stuff
*
*******************************************************************************/

#include "freemaster.h"
#include "freemaster_private.h"
#include "MQX.h"




void FMSTR_CopyMemory(uint8_t *nDestAddr, uint8_t *nSrcAddr, uint8_t nSize)
{
  uint8_t *ps = (uint8_t *)nSrcAddr;
  uint8_t *pd = (uint8_t *)nDestAddr;

  while (nSize--) *pd++ = *ps++;
}

/**************************************************************************//*!
*
* @brief  Write-into the communication buffer memory
*
* @param  pDestBuff - pointer to destination memory in communication buffer
* @param  nSrcAddr  - source memory address
* @param  nSize     - buffer size (always in bytes)
*
* @return This function returns a pointer to next byte in comm. buffer
*
******************************************************************************/

uint8_t*  FMSTR_CopyToBuffer(uint8_t *pDestBuff, uint8_t *nSrcAddr, uint8_t nSize)
{
  uint8_t *ps = (uint8_t *)nSrcAddr;
  uint8_t *pd = (uint8_t *)pDestBuff;

  _int_disable();
  while (nSize--)
  {
    *pd++ = *ps++;
  }
  _int_enable();

  return (uint8_t *)pd;
}

/**************************************************************************//*!
*
* @brief  Read-out memory from communication buffer
*
* @param  nDestAddr - destination memory address
* @param  pSrcBuff  - pointer to source memory in communication buffer
* @param  nSize     - buffer size (always in bytes)
*
* @return This function returns a pointer to next byte in comm. buffer
*
******************************************************************************/

uint8_t*  FMSTR_CopyFromBuffer(uint8_t *nDestAddr, uint8_t *pSrcBuff, uint8_t nSize)
{
  uint8_t *ps = (uint8_t *)pSrcBuff;
  uint8_t *pd = (uint8_t *)nDestAddr;

  while (nSize--)
  {
    *pd++ = *ps++;
  }

  return (uint8_t *)ps;
}


/**************************************************************************//*!
*
* @brief  Read-out memory from communication buffer, perform AND-masking
*
* @param  nDestAddr - destination memory address
* @param  pSrcBuff  - source memory in communication buffer, mask follows data
* @param  nSize     - buffer size (always in bytes)
*
******************************************************************************/

void FMSTR_CopyFromBufferWithMask(uint8_t *nDestAddr, uint8_t *pSrcBuff, uint8_t nSize)
{
  uint8_t *ps = (uint8_t *)pSrcBuff;
  uint8_t *pd = (uint8_t *)nDestAddr;
  uint8_t *pm = ps + nSize;
  uint8_t mask, stmp, dtmp;

  while (nSize--)
  {
    mask = *pm++;
    stmp = *ps++;
    dtmp = *pd;

    /* perform AND-masking */
    stmp = (uint8_t)((stmp & mask) | (dtmp & ~mask));

    /* put the result back */
    *pd++ = stmp;
  }
}

