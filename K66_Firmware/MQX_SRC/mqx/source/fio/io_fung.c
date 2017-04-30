
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains the function for ungetting a character.
*
*
*END************************************************************************/
#include "mqx_cnfg.h"
#if MQX_USE_IO_OLD

#include "mqx.h"
#include "fio.h"
#include "fio_prv.h"
#include "io.h"
#include "io_prv.h"

/*!
 * \brief Pushes back a character.
 *  
 * Pushed character will be returned on next read. Only 1 pushback character is
 * allowed.
 * 
 * \param[in] character The character to return.
 * \param[in] file_ptr  The stream to push character back to.
 * 
 * \return Pushed back character.
 * \return IO_EOF (Failure.)  
 */ 
_mqx_int _io_fungetc
   (
      _mqx_int     character,
      MQX_FILE_PTR file_ptr
   )
{ /* Body */

#if MQX_CHECK_ERRORS
   if (file_ptr == NULL) {
      return(IO_EOF);
   } /* Endif */
#endif

   if (file_ptr->HAVE_UNGOT_CHARACTER) {
      return(IO_EOF);
   } /* Endif */

   if (character != IO_EOF) {
       /* Clear EOF flag */
       file_ptr->UNGOT_CHARACTER      = character;
       file_ptr->HAVE_UNGOT_CHARACTER = TRUE;
       file_ptr->FLAGS &= ~((_mqx_uint) IO_FLAG_AT_EOF);
   }
   return(character);

} /* Endbody */

#endif // MQX_USE_IO_OLD
