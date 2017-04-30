/***************************************************************************//*!
*
* @file   freemaster_cfg.h
*
* @brief  FreeMASTER Serial Communication Driver configuration file
*
*******************************************************************************/

#ifndef __FREEMASTER_CFG_H
#define __FREEMASTER_CFG_H


/*****************************************************************************
* Select communication interface 
******************************************************************************/

#define FMSTR_DISABLE          0    /* To disable all FreeMASTER functionalities */
#define FMSTR_USE_USB_CDC      1   

/******************************************************************************
* Input/output communication buffer size
******************************************************************************/

#define FMSTR_COMM_BUFFER_SIZE 0    /* set to 0 for "automatic" */

/*****************************************************************************
* Support for Application Commands 
******************************************************************************/

#define FMSTR_USE_APPCMD       1    /* enable/disable App.Commands support */
#define FMSTR_APPCMD_BUFF_SIZE 32   /* App.Command data buffer size */
#define FMSTR_MAX_APPCMD_CALLS 10   /* how many app.cmd callbacks? (0=disable) */

/*****************************************************************************
* Oscilloscope support
******************************************************************************/

#define FMSTR_USE_SCOPE        1    /* enable/disable scope support */
#define FMSTR_MAX_SCOPE_VARS   8    /* max. number of scope variables (2..8) */

/*****************************************************************************
* Поддержка протокола к блоку MATLAB SFIO
******************************************************************************/

#define FMSTR_USE_SFIO         0 

/*****************************************************************************
* Поддержка протокола каналов PIPE
******************************************************************************/

#define FMSTR_USE_PIPES        0 


/*****************************************************************************
* Recorder support
******************************************************************************/

#define FMSTR_USE_RECORDER     1    /* enable/disable recorder support */
#define FMSTR_MAX_REC_VARS     8    /* max. number of recorder variables (2..8) */
#define FMSTR_REC_OWNBUFF      0    /* use user-allocated rec. buffer (1=yes) */

/* built-in recorder buffer (use when FMSTR_REC_OWNBUFF is 0) */
#define FMSTR_REC_BUFF_SIZE    32000// 65535 /* built-in buffer size */

// Интервал сэмплирования рекордера , не указываем поскольку частота сэмплирование не укладывается ы целое число микросекунд 
#define FMSTR_REC_TIMEBASE     FMSTR_REC_BASE_MICROSEC(0) /* 0 = "unknown" */

#define FMSTR_REC_FLOAT_TRIG   0    /* enable/disable floating point triggering */

/*****************************************************************************
* Target-side address translation (TSA)
******************************************************************************/

#define FMSTR_USE_TSA          1    /* enable TSA functionality */
#define FMSTR_USE_TSA_SAFETY   1    /* enable access to TSA variables only */
#define FMSTR_USE_TSA_INROM    1    /* TSA tables declared as const (put to ROM) */
#define FMSTR_USE_TSA_DYNAMIC  1    /* enable dynamic TSA table */

/*****************************************************************************
* Enable/Disable read/write memory commands
******************************************************************************/

#define FMSTR_USE_READMEM      1    /* enable read memory commands */
#define FMSTR_USE_WRITEMEM     1    /* enable write memory commands */
#define FMSTR_USE_WRITEMEMMASK 1    /* enable write memory bits commands */

/*****************************************************************************
* Enable/Disable read/write variable commands (a bit faster than Read Mem)
******************************************************************************/

#define FMSTR_USE_READVAR      1    /* enable read variable fast commands */
#define FMSTR_USE_WRITEVAR     1    /* enable write variable fast commands */
#define FMSTR_USE_WRITEVARMASK 1    /* enable write variable bits fast commands */


#endif /* __FREEMASTER_CFG_H */
