/*HEADER**********************************************************************
*
* Copyright 2009 Freescale Semiconductor, Inc.
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
*   configure a wireless network interface.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>

#if !PLATFORM_SDK_ENABLED

#if MQX_USE_IO_OLD
#include <fio.h>
#else
#include <stdio.h>
#endif
#include <rtcs.h>
#include <string.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "ip_prv.h"
#include "ipcfg.h"
#include "ipcfg_prv.h"
#include "enet_wifi.h"
#include "iwcfg.h"

#ifdef BSP_ENET_DEVICE_COUNT
#if  (BSP_ENET_DEVICE_COUNT > 0) 

RSSI_QUANTA g_rssiTable[MAX_RSSI_TABLE_SZ] = {    
                                                  {200,181},
                                                  {180,161},
                                                  {160,141},
                                                  {140,121},
                                                  {121,100} 
                                             };

#define RSSI_TBL_ENTRY_MIN(x)    g_rssiTable[x].min
#define RSSI_TBL_ENTRY_MAX(x)    g_rssiTable[x].max

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : iwcfg_set_ssid()
* Returned Value  : ENET_OK if ESSID set successfully else ERROR CODE
* Comments        : Sets the ESSID. ESSID comes into effect only when 
*                   user commits his changes.
*
*END*-----------------------------------------------------------------*/

uint32_t iwcfg_set_essid
    (
        uint32_t dev_num,
        char *essid
    )
{
    _rtcs_if_handle ihandle;
    ENET_ESSID  inout_param;
    uint32_t error;
    bool dev_status;
    
    if (dev_num < IPCFG_DEVICE_COUNT)
    {
        ihandle = ipcfg_get_ihandle(dev_num);
        
        if (ihandle == NULL)
        {
            printf("Initialize Wifi Device Using ipconfig\n");
            return ENET_ERROR;
        }
        if (!essid)
          inout_param.flags = 0;
        else
         inout_param.flags = 1;
        
        inout_param.essid = essid;
        inout_param.length = strlen(essid);
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_MEDIACTL_IS_INITIALIZED,&dev_status);
        if (ENET_OK != error)
        {
            return error;
        }
        if (dev_status == FALSE)
        {
            return ENET_ERROR;
        }

        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_SET_MEDIACTL_ESSID,&inout_param);
        if (ENET_OK != error)
        {
            return error;
        }
        return ENET_OK;
    }
    printf("IWCONFIG_ERROR: Invalid Device number\n");
    return ENETERR_INVALID_DEVICE;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : iwcfg_get_ssid()
* Returned Value  : ENET_OK if ESSID set successfully else ERROR CODE
* Comments        : Sets the ESSID. ESSID comes into effect only when 
*                   user commits his changes.
*
*END*-----------------------------------------------------------------*/

uint32_t iwcfg_get_essid
    (
        uint32_t dev_num,
        char *essid /*[OUT]*/
    )
{
    _rtcs_if_handle ihandle;
    ENET_MEDIACTL_PARAM  inout_param;
    uint32_t error;
    bool dev_status;
    
    
    if (dev_num < IPCFG_DEVICE_COUNT)
    {
        ihandle = ipcfg_get_ihandle(dev_num);
        
        if (ihandle == NULL)
        {
            printf("Initialize Wifi Device Using ipconfig\n");
            return ENET_ERROR;
        }

        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_MEDIACTL_IS_INITIALIZED,&dev_status);
        if (ENET_OK != error)
        {
            return error;
        }
        if (dev_status == FALSE)
        {
            return ENET_ERROR;
        }

        inout_param.data = (void *)essid;
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_GET_MEDIACTL_ESSID,&inout_param);
        if (ENET_OK != error)
        {
            return error;
        }
        
        if (!inout_param.flags)
            return ENET_ERROR;
        
        essid[inout_param.length] = '\0';
        return ENET_OK;
    }
    printf("IWCONFIG_ERROR: Invalid Device number\n");
    return ENETERR_INVALID_DEVICE;
}



/*FUNCTION*--------------------------------------------------------------------
*
* Function Name   : iwcfg_commit()
* Returned Value  : ENET_OK if commit is successfull else ERROR CODE
* Comments        : Commits the requested change. It also associates
*                   with the requested AP with SSID set by iwcfg_set_essid 
*
*END*------------------------------------------------------------------------*/

uint32_t iwcfg_commit
    (
        uint32_t dev_num
    )
{
    _rtcs_if_handle ihandle;
    uint32_t error;
    bool dev_status;
    
    if (dev_num < IPCFG_DEVICE_COUNT)
    {
        ihandle = ipcfg_get_ihandle(dev_num);
        if (ihandle == NULL)
        {
            printf("Initialize Wifi Device Using ipconfig\n");
            return ENET_ERROR;
        }        
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_MEDIACTL_IS_INITIALIZED,&dev_status);
        if (ENET_OK != error)
        {
            return error;
        }
        if (dev_status == FALSE)
        {
            return ENET_ERROR;
        }

        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_SET_MEDIACTL_COMMIT,NULL);
        if (ENET_OK != error)
        {
            return error;
        }
        return ENET_OK;
    }
    
    return ENETERR_INVALID_DEVICE;
}

/*FUNCTION*--------------------------------------------------------------------
*
* Function Name   : iwcfg_set_mode()
* Returned Value  : ENET_CODE if mode set successfull else ERROR CODE
* Comments        : Sets the Mode for Wifi Device. 
*
*END*------------------------------------------------------------------------*/

uint32_t iwcfg_set_mode
    (
        uint32_t dev_num,
        char *mode
    )
{
    _rtcs_if_handle ihandle;
    uint32_t error;
    uint32_t inout_param;
    bool dev_status;
 
    if (!strcmp(mode,"managed")) 
    {
        inout_param = ENET_MEDIACTL_MODE_INFRA;
    }
    else if (!strcmp(mode,"adhoc")) 
    {
        inout_param = ENET_MEDIACTL_MODE_ADHOC;
    }
    else
    {
        printf("Invalid Mode parameter %s. Refer help\n",mode);
        return ENET_ERROR;
    }
        
    if (dev_num < IPCFG_DEVICE_COUNT)
    {
        ihandle = ipcfg_get_ihandle(dev_num);
        if (ihandle == NULL)
        {
            printf("Initialize Wifi Device Using ipconfig\n");
            return ENET_ERROR;
        }        
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_MEDIACTL_IS_INITIALIZED,&dev_status);
        if (ENET_OK != error)
        {
            return error;
        }
        if (dev_status == FALSE)
        {
            return ENET_ERROR;
        }
        
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_SET_MEDIACTL_MODE,&inout_param);
        if (ENET_OK != error)
        {
            return error;
        }
        return ENET_OK;
    }
    
    return ENETERR_INVALID_DEVICE;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : iwcfg_get_mode()
* Returned Value  : ENET_OK if ESSID set successfully else ERROR CODE
* Comments        : Sets the ESSID. ESSID comes into effect only when 
*                   user commits his changes.
*
*END*-----------------------------------------------------------------*/

uint32_t iwcfg_get_mode
    (
        uint32_t dev_num,
        char *mode /*[OUT]*/
    )
{
    _rtcs_if_handle ihandle;
    uint32_t  inout_param;
    uint32_t error;
    bool dev_status;
    
    if (dev_num < IPCFG_DEVICE_COUNT)
    {
        ihandle = ipcfg_get_ihandle(dev_num);
        
        if (ihandle == NULL)
        {
            printf("Initialize Wifi Device Using ipconfig\n");
            return ENET_ERROR;
        }

        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_MEDIACTL_IS_INITIALIZED,&dev_status);
        if (ENET_OK != error)
        {
            return error;
        }
        if (dev_status == FALSE)
        {
            return ENET_ERROR;
        }

        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_GET_MEDIACTL_MODE,&inout_param);
        if (ENET_OK != error)
        {
            return error;
        }
        
        if (inout_param == ENET_MEDIACTL_MODE_INFRA)  
           strcpy(mode,"managed");
        else if (inout_param == ENET_MEDIACTL_MODE_ADHOC)
           strcpy(mode,"adhoc");
        else
        {
           mode = NULL;
           return ENET_ERROR;   
        }
        
        return ENET_OK;
    }
    printf("IWCONFIG_ERROR: Invalid Device number\n");
    return ENETERR_INVALID_DEVICE;
}
/*FUNCTION*--------------------------------------------------------------------
*
* Function Name   : iwcfg_set_wep_key()
* Returned Value  : ENET_OK if mode set successfull else ERROR CODE
* Comments        : Sets the Mode for Wifi Device. 
*
*END*------------------------------------------------------------------------*/

uint32_t iwcfg_set_wep_key 
    (
        uint32_t dev_num,
        char *wep_key,
        uint32_t key_len,
        uint32_t key_index
        
    )
{
    _rtcs_if_handle ihandle;
    uint32_t error;
    bool dev_status;
    uint32_t flags = 0;
    ENET_MEDIACTL_PARAM  inout_param;

   if (dev_num < IPCFG_DEVICE_COUNT)
    {
        ihandle = ipcfg_get_ihandle(dev_num);
        if (ihandle == NULL)
        {
            printf("Initialize Wifi Device Using ipconfig\n");
            return ENET_ERROR;
        }        
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_MEDIACTL_IS_INITIALIZED,&dev_status);
        if (ENET_OK != error)
        {
            return error;
        }
        if (dev_status == FALSE)
        {
            return ENET_ERROR;
        }
        
        if (wep_key == NULL)
         flags |= ENET_MEDIACTL_ENCODE_DISABLED|ENET_MEDIACTL_ENCODE_MODE;
        else
         flags |= ENET_MEDIACTL_ENCODE_ENABLED;
        
        flags |= (key_index & 0xFF); 
        inout_param.data = wep_key;
        inout_param.length = key_len;
        inout_param.flags = flags;
        
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_SET_MEDIACTL_ENCODE,&inout_param);
        if (ENET_OK != error)
        {
            return error;
        }
        return ENET_OK;
    }   
    return ENETERR_INVALID_DEVICE;  
}

/*FUNCTION*--------------------------------------------------------------------
*
* Function Name   : iwcfg_get_wep_key()
* Returned Value  : ENET_OK if mode set successfull else ERROR CODE
* Comments        : Sets the Mode for Wifi Device. 
*
*END*------------------------------------------------------------------------*/

uint32_t iwcfg_get_wep_key 
    (
        uint32_t dev_num,
        char *wep_key,
        uint32_t *key_index
  
    )
{
    _rtcs_if_handle ihandle;
    uint32_t error;
    bool dev_status;
    ENET_MEDIACTL_PARAM  inout_param;

   if (dev_num < IPCFG_DEVICE_COUNT)
    {
        ihandle = ipcfg_get_ihandle(dev_num);
        if (ihandle == NULL)
        {
            printf("Initialize Wifi Device Using ipconfig\n");
            return ENET_ERROR;
        }        
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_MEDIACTL_IS_INITIALIZED,&dev_status);
        if (ENET_OK != error)
        {
            return error;
        }
        if (dev_status == FALSE)
        {
            return ENET_ERROR;
        }
        
        inout_param.data = (void *)wep_key;
                
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_GET_MEDIACTL_ENCODE,&inout_param);
        if (ENET_OK != error)
        {
            return error;
        }
        wep_key[inout_param.length] = '\0';
        if (key_index)
           *key_index = inout_param.flags & 0xFF;
        return ENET_OK;
    }   
    return ENETERR_INVALID_DEVICE;  
}

/*FUNCTION*--------------------------------------------------------------------
*
* Function Name   : iwcfg_get_passphrase()
* Returned Value  : ENET_OK if mode set successfull else ERROR CODE
* Comments        : Sets the Mode for Wifi Device. 
*
*END*------------------------------------------------------------------------*/

uint32_t iwcfg_get_passphrase 
    (
        uint32_t dev_num,
        char *passphrase
         
    )
{
    _rtcs_if_handle ihandle;
    uint32_t error;
    bool dev_status;
    ENET_MEDIACTL_PARAM  inout_param;

   if (dev_num < IPCFG_DEVICE_COUNT)
    {
        ihandle = ipcfg_get_ihandle(dev_num);
        if (ihandle == NULL)
        {
            printf("Initialize Wifi Device Using ipconfig\n");
            return ENET_ERROR;
        }        
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_MEDIACTL_IS_INITIALIZED,&dev_status);
        if (ENET_OK != error)
        {
            return error;
        }
        if (dev_status == FALSE)
        {
            return ENET_ERROR;
        }
        
        inout_param.data = (void *)passphrase;
                
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_GET_MEDIACTL_PASSPHRASE,&inout_param);
        if (ENET_OK != error)
        {
            return error;
        }
        
        passphrase[inout_param.length] = '\0';
        
        return ENET_OK;
    }   
    return ENETERR_INVALID_DEVICE;  
}
/*FUNCTION*--------------------------------------------------------------------
*
* Function Name   : iwcfg_set_wep_key()
* Returned Value  : ENET_OK if mode set successfull else ERROR CODE
* Comments        : Sets the Mode for Wifi Device. 
*
*END*------------------------------------------------------------------------*/

uint32_t iwcfg_set_sec_type 
    (
        uint32_t dev_num,
        char  *sec_type
    )
{
    _rtcs_if_handle ihandle;
    uint32_t error,inout_param;
    bool dev_status;

   if (strcmp(sec_type,"none") == 0)
   {
      inout_param = ENET_MEDIACTL_SECURITY_TYPE_NONE;
   }
   else if (strcmp(sec_type,"wep") == 0)
   {
     inout_param = ENET_MEDIACTL_SECURITY_TYPE_WEP;
   }
   else if (strcmp(sec_type,"wpa") == 0)
   {
     inout_param = ENET_MEDIACTL_SECURITY_TYPE_WPA;
   }
   else if (strcmp(sec_type,"wpa2") == 0)
   {
     inout_param = ENET_MEDIACTL_SECURITY_TYPE_WPA2;
   }
   else
   {
     printf("%s sectype passed is incorrect. refer help\n",sec_type);
     return ENET_ERROR;
   }
   
   if (dev_num < IPCFG_DEVICE_COUNT)
    {
        ihandle = ipcfg_get_ihandle(dev_num);
        if (ihandle == NULL)
        {
            printf("Initialize Wifi Device Using ipconfig\n");
            return ENET_ERROR;
        }        
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_MEDIACTL_IS_INITIALIZED,&dev_status);
        if (ENET_OK != error)
        {
            return error;
        }
        if (dev_status == FALSE)
        {
            return ENET_ERROR;
        }
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_SET_MEDIACTL_SEC_TYPE,&inout_param);
        if (ENET_OK != error)
        {
            return error;
        }
        return ENET_OK;
    }   
    return ENETERR_INVALID_DEVICE;  
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : iwcfg_get_sectype()
* Returned Value  : ENET_OK if ESSID set successfully else ERROR CODE
* Comments        : Sets the ESSID. ESSID comes into effect only when 
*                   user commits his changes.
*
*END*-----------------------------------------------------------------*/

uint32_t iwcfg_get_sectype
    (
        uint32_t dev_num,
        char *sectype /*[OUT]*/
    )
{
    _rtcs_if_handle ihandle;
    uint32_t  inout_param;
    uint32_t error;
    bool dev_status;
    
    if (dev_num < IPCFG_DEVICE_COUNT)
    {
        ihandle = ipcfg_get_ihandle(dev_num);
        
        if (ihandle == NULL)
        {
            printf("Initialize Wifi Device Using ipconfig\n");
            return ENET_ERROR;
        }

        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_MEDIACTL_IS_INITIALIZED,&dev_status);
        if (ENET_OK != error)
        {
            return error;
        }
        if (dev_status == FALSE)
        {
            return ENET_ERROR;
        }

        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_GET_MEDIACTL_SEC_TYPE,&inout_param);
        if (ENET_OK != error)
        {
            return error;
        }
        
        if (inout_param == ENET_MEDIACTL_SECURITY_TYPE_NONE)  
           strcpy(sectype,"none");
        else if (inout_param == ENET_MEDIACTL_SECURITY_TYPE_WEP)
           strcpy(sectype,"wep");
        else if (inout_param == ENET_MEDIACTL_SECURITY_TYPE_WPA)
           strcpy(sectype,"wpa");
        else if (inout_param == ENET_MEDIACTL_SECURITY_TYPE_WPA2)
           strcpy(sectype,"wpa2");
        else
        {
           sectype = NULL;
           return ENET_ERROR;   
        }
        
        return ENET_OK;
    }
    printf("IWCONFIG_ERROR: Invalid Device number\n");
    return ENETERR_INVALID_DEVICE;
}
/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : iwcfg_set_passphrase()
* Returned Value  : ENET_OK if ESSID set successfully else ERROR CODE
* Comments        : Sets the ESSID. ESSID comes into effect only when 
*                   user commits his changes.
*
*END*-----------------------------------------------------------------*/

uint32_t iwcfg_set_passphrase
    (
        uint32_t dev_num,
        char *passphrase
    )
{
    _rtcs_if_handle ihandle;
    uint32_t error;
    bool dev_status;
    ENET_MEDIACTL_PARAM param;
    
    if (dev_num < IPCFG_DEVICE_COUNT)
    {
        ihandle = ipcfg_get_ihandle(dev_num);
        if (ihandle == NULL)
        {
            printf("Initialize Wifi Device Using ipconfig\n");
            return ENET_ERROR;
        }        
        param.data = passphrase;
        param.length = strlen(passphrase);
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_MEDIACTL_IS_INITIALIZED,&dev_status);
        if (ENET_OK != error)
        {
            return error;
        }
        if (dev_status == FALSE)
        {
            return ENET_ERROR;
        }
        
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_SET_MEDIACTL_PASSPHRASE,&param);
        if (ENET_OK != error)
        {
            return error;
        }
        return ENET_OK;
    }
    printf("IWCONFIG_ERROR: Invalid Device number\n");
    return ENETERR_INVALID_DEVICE;
}

/*FUNCTION*--------------------------------------------------------------------
*
* Function Name   : iwcfg_set_wep_key()
* Returned Value  : ENET_OK if mode set successfull else ERROR CODE
* Comments        : Sets the Mode for Wifi Device. 
*
*END*------------------------------------------------------------------------*/

uint32_t iwcfg_set_power 
    (
        uint32_t dev_num,
        uint32_t pow_val,
        uint32_t flags
        
    )
{
    _rtcs_if_handle ihandle;
    uint32_t error;
    bool dev_status;
    ENET_MEDIACTL_PARAM  inout_param;

   if (dev_num < IPCFG_DEVICE_COUNT)
    {
        ihandle = ipcfg_get_ihandle(dev_num);
        if (ihandle == NULL)
        {
            printf("Initialize Wifi Device Using ipconfig\n");
            return ENET_ERROR;
        }        
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_MEDIACTL_IS_INITIALIZED,&dev_status);
        if (ENET_OK != error)
        {
            return error;
        }
        if (dev_status == FALSE)
        {
            return ENET_ERROR;
        }
        
        inout_param.value = pow_val;
        inout_param.disabled = flags;
        inout_param.flags = ENET_MEDIACTL_POWER_PERIOD;
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_SET_MEDIACTL_POWER,&inout_param);
        if (ENET_OK != error)
        {
            return error;
        }
        return ENET_OK;
    }   
    return ENETERR_INVALID_DEVICE;  
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : iwcfg_set_scan()
* Returned Value  : ENET_OK if ESSID set successfully else ERROR CODE
* Comments        : Sets the ESSID. ESSID comes into effect only when 
*                   user commits his changes.
*
*END*-----------------------------------------------------------------*/

uint32_t iwcfg_set_scan
    (
        uint32_t dev_num,
        char *ssid
    )
{
    _rtcs_if_handle ihandle;
    uint32_t error;
    bool dev_status;
    uint32_t i=0;
    ENET_SCAN_LIST  param;
    
    if (dev_num < IPCFG_DEVICE_COUNT)
    {
        ihandle = ipcfg_get_ihandle(dev_num);
        if (ihandle == NULL)
        {
            printf("Initialize Wifi Device Using ipconfig\n");
            return ENET_ERROR;
        }        
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_MEDIACTL_IS_INITIALIZED,&dev_status);
        if (ENET_OK != error)
        {
            return error;
        }
        if (dev_status == FALSE)
        {
            return ENET_ERROR;
        }
        
        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_SET_MEDIACTL_SCAN,NULL);
        if (ENET_OK != error)
        {
            return error;
        }

        error = ENET_mediactl (((IP_IF_PTR)ihandle)->HANDLE,ENET_GET_MEDIACTL_SCAN,&param);
        if (ENET_OK != error)
        {
            return error;
        }
   
        for (i = 0;i<param.num_scan_entries;i++)
        {
            uint8_t j = MAX_RSSI_TABLE_SZ-1;
            
            printf("\n\r");
            if (param.scan_info_list[i].ssid_len == 1)
               printf("ssid = SSID Not available\n");
            else
               printf ("ssid = %s\n",param.scan_info_list[i].ssid);
            printf ("bssid = %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",param.scan_info_list[i].bssid[0],param.scan_info_list[i].bssid[1],param.scan_info_list[i].bssid[2],param.scan_info_list[i].bssid[3],param.scan_info_list[i].bssid[4],param.scan_info_list[i].bssid[5]);
            printf ("channel = %d\n",param.scan_info_list[i].channel);
            printf("strength = ");
             do {    
      
                if ( RSSI_TBL_ENTRY_MIN(j) < param.scan_info_list[i].rssi )
                  printf("#");
                else
                  printf(".");
                  
            } while ( j-- > 0 );
            printf("\nindicator = %d",param.scan_info_list[i].rssi);
            printf("\n\r");
        }
        return ENET_OK;
    }
    printf("IWCONFIG_ERROR: Invalid Device number\n");
    return ENETERR_INVALID_DEVICE;
}

#endif /* (BSP_ENET_DEVICE_COUNT > 0) */
#endif /* BSP_ENET_DEVICE_COUNT */

#endif /*!PLATFORM_SDK_ENABLED*/

/*EOF*/
