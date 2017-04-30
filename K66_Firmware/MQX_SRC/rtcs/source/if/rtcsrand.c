/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   This file contains the implementation of a random
*   number generator.
*
*
*END************************************************************************/

#include <rtcs.h>


#if RTCSCFG_USE_KISS_RNG 

/*
 * Probably the simplest RNG to pass all of the common RNG tests (and many 
 * others) is the KISS generator proposed by G. Marsaglia (called KISS99 in 
 * the L’Ecuyer paper).
 *
 * This generator combines 3 different simple generators and has a period of 
 * around 10^38, i.e. it will start repeating the same sequence of numbers 
 * after that many calls. Plenty for pretty much any conceivable application. 
 * 
 * Note that you need to set the seed values (at least x, y and z need to be 
 * changed) to random starting values else you will always generate the same 
 * sequence of numbers in your program. Avoid setting the seeds to zero or 
 * small numbers in general. Try to choose large complex seed values.
 */
 
static uint32_t seed_x = 123456789;
static uint32_t seed_y = 362436000;
static uint32_t seed_z = 521288629;
static uint32_t seed_c = 7654321;

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_gen_seed
* Returned Values : uint32_t
* Comments        :
*     Add randomness to the seed.
*
*END*-----------------------------------------------------------------*/

static uint32_t kiss_gen_seed(x, y)
{
   x = x + y;
   x = (x << 15) | (x >> 17);
   return (x & 0xFFFFFFFFUL);
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_rand_seed
* Returned Values : uint32_t
* Comments        :
*     Seed the KISS RNG.  Y _must_ not be zero, but good practice to make 
*     sure they are all not zero.
*
*END*-----------------------------------------------------------------*/

void RTCS_rand_seed
   (
      uint32_t newseed
   )
{ /* Body */
   do {
      seed_x = kiss_gen_seed(seed_x, newseed);
   } while (seed_x == 0);

   do {
      seed_y = kiss_gen_seed(seed_y, newseed);
   } while (seed_y == 0);
   
   do {
      seed_z = kiss_gen_seed(seed_z, newseed);
   } while (seed_z == 0);
   
   /* Don’t really need to seed c as well but if you really want to,
      make sure it is less than 698769069 */
   do {
      seed_c = (kiss_gen_seed(seed_c, newseed) % 698769069);
   } while (seed_c == 0);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_rand
* Returned Values : uint32_t
* Comments        :
*     Output 32 random bits.  Based on George Marsaglia's KISS RNG
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_rand
   (
      void
   )
{ /* Body */
  uint64_t t, a=698769069LL;

  seed_x = 69069 * seed_x + 12345;
  seed_y ^= (seed_y<<13); 
  seed_y ^= (seed_y>>17); 
  seed_y ^= (seed_y<<5);
  t = a * seed_z + seed_c; 
  seed_c = (t>>32);
  
  return seed_x + seed_y + (seed_z = t);

} /* Endbody */

#else

// Original RTCS RNG 

#define P 0x6C77
#define Q 0x12E1B

static uint32_t seed   = 0;
static uint32_t reseed = 0;
static uint32_t state  = 4;


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_rand_seed
* Returned Values : void
* Comments        :
*     Add randomness to the seed.
*
*END*-----------------------------------------------------------------*/

void RTCS_rand_seed
   (
      uint32_t newseed
   )
{ /* Body */
   uint32_t realseed = seed;

   realseed = realseed + newseed;
   realseed = (realseed << 15) | (realseed >> 17);
   realseed = realseed & 0xFFFFFFFFUL;

   seed = realseed;
   reseed = 1;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_rand
* Returned Values : uint32_t
* Comments        :
*     Output 32 random bits.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_rand
   (
      void
   )
{ /* Body */
   uint32_t i,j;
   uint32_t pq = P*Q;
   uint32_t square;
   uint32_t randnum;

   if (reseed) {
      uint32_t realseed = seed ^ state;
      uint32_t resp, resq;
      while (realseed >= pq) {
         realseed -= pq;
      } /* Endwhile */
      for (;;) {
         resp = realseed % P;
         resq = realseed % Q;
         if ((resp != 0) && (resp != 1) && (resp != P-1)
          && (resq != 0) && (resq != 1) && (resq != Q-1)) {
            break;
         } /* Endif */
         realseed += 3;
         if (realseed >= pq) realseed -= pq;
      } /* Endfor */
      state = realseed;
      reseed = 0;
   } /* Endif */

   randnum = 0;
   for (i=0; i<32; i++) {
      /* square = state*state mod pq */
      square = 0;
      for (j=0x40000000UL; j; j>>=1) {
         square <<= 1;
         if (square >= pq) square -= pq;
         if (state & j)    square += state;
         if (square >= pq) square -= pq;
      } /* Endfor */
      state = square;
      randnum = (randnum << 1) | (state & 1);
   } /* Endfor */

   return randnum;
} /* Endbody */

#endif

/* EOF */
