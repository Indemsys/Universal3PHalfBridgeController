#ifndef __mfs_cnfg_h__
#define __mfs_cnfg_h__
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
*   This file contains the structure definitions and constants for a
*   user who will be compiling programs that will use the Embedded MS-DOS
*   File System (MFS)
*
*
*END************************************************************************/

/* 
** Can be modified to customize MFS 
*/

/***************************************
**
** Code and Data configuration options
**
** MGCT: <category name="Code and Data configuration options">
**
*/

/*
** Define MFSCFG_MINIMUM_FOOTPRINT to reducte MFS memory requirements.
** MGCT: <option type="bool"/>
*/
#ifndef MFSCFG_MINIMUM_FOOTPRINT
   #define MFSCFG_MINIMUM_FOOTPRINT 0
#endif

/*
** MGCT: <option type="bool"/>
*/
#ifndef MFSCFG_READ_ONLY
   #define MFSCFG_READ_ONLY 0
#endif

/*
** Deprecated and not used anymore.
** MGCT: <option type="bool"/>
*/
#ifndef MFSCFG_READ_ONLY_CHECK
   #define MFSCFG_READ_ONLY_CHECK 1
#endif

/*
** Deprecated and not used anymore.
** MGCT: <option type="bool"/>
**
*/
#ifndef MFSCFG_READ_ONLY_CHECK_ALWAYS
   #define MFSCFG_READ_ONLY_CHECK_ALWAYS 0
#endif

/*
** MGCT: <option type="bool"/>
*/
#ifndef MFSCFG_CALCULATE_FREE_SPACE_ON_OPEN
   #define MFSCFG_CALCULATE_FREE_SPACE_ON_OPEN 0 
#endif

/*
** Only used if formatting. Value must be 1 or 2. 2 is the standard.
** MGCT: <option type="list">
** <item name="1" value="1"/>
** <item name="2" value="2"/>
** </option>
*/
#ifndef MFSCFG_NUM_OF_FATS
   #define MFSCFG_NUM_OF_FATS 1
#endif
 
/*
** Number of slots to of the sectors cache. Minimum value of 2 required.
** MGCT: <option type="number"/>
*/           
#ifndef MFSCFG_SECTOR_CACHE_SIZE
   #if MFSCFG_MINIMUM_FOOTPRINT
      #define MFSCFG_SECTOR_CACHE_SIZE 2
   #else
      #define MFSCFG_SECTOR_CACHE_SIZE 4
   #endif
#endif

/* 
** The amount of times MFS will attempt to read or write to the device 
** unsuccessfully before reporting an error.
** MGCT: <option type="number"/>
*/
#ifndef MFSCFG_MAX_WRITE_RETRIES
   #define MFSCFG_MAX_WRITE_RETRIES    1
#endif

/*
** MGCT: <option type="number"/>
*/
#ifndef MFSCFG_MAX_READ_RETRIES
   #define MFSCFG_MAX_READ_RETRIES  1
#endif

/*
** MGCT: <option type="number"/>
*/
#ifndef MFSCFG_HANDLE_INITIAL
   #if MFSCFG_MINIMUM_FOOTPRINT
      #define MFSCFG_HANDLE_INITIAL 4
   #else
      #define MFSCFG_HANDLE_INITIAL 16
   #endif
#endif

/*
** MGCT: <option type="number"/>
*/
#ifndef MFSCFG_HANDLE_GROW
   #if MFSCFG_MINIMUM_FOOTPRINT
      #define MFSCFG_HANDLE_GROW 4 
   #else
      #define MFSCFG_HANDLE_GROW 16
   #endif
#endif

/*
** MGCT: <option type="number"/>
*/
#ifndef MFSCFG_HANDLE_MAX
   #if MFSCFG_MINIMUM_FOOTPRINT
      #define MFSCFG_HANDLE_MAX  0 
   #else
      #define MFSCFG_HANDLE_MAX  0
   #endif
#endif

/*
** MGCT: <option type="number"/>
*/
#ifndef MFSCFG_FIND_TEMP_TRIALS
   #define MFSCFG_FIND_TEMP_TRIALS      300
#endif

/*
** MGCT: <option type="bool"/>
*/
#ifndef MFSCFG_ENABLE_FORMAT
    #if MFSCFG_MINIMUM_FOOTPRINT
        #define MFSCFG_ENABLE_FORMAT 0
    #else
        #define MFSCFG_ENABLE_FORMAT 1
    #endif
#endif

/*
** MGCT: <option type="bool"/>
*/
#ifndef MFSCFG_USE_MUTEX
    #define MFSCFG_USE_MUTEX 0
#endif

/** MGCT: </category> */

#endif
