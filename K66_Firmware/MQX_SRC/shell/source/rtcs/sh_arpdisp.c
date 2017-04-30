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
*   Example using RTCS Library.
*
*
*END************************************************************************/

#include <mqx.h>
#include "shell.h"
#if SHELLCFG_USES_RTCS
#include <rtcs.h>
#include <enet.h>
#include <ipcfg.h>
#include "sh_rtcs.h"
#include "sh_enet.h"

#ifdef BSP_ENET_DEVICE_COUNT
#if  (BSP_ENET_DEVICE_COUNT > 0)


extern void ARP_display_if_table(_rtcs_if_handle   ihandle);
extern uint32_t RTCS_arp_delete (_rtcs_if_handle    *ihandle,_ip_address paddr );

/*TASK*-----------------------------------------------------------------
*
* Function Name  : Shell_arpdisp
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/

int32_t Shell_arpdisp(int32_t argc, char *argv[] )
{ /* Body */
   bool              print_usage, shorthelp = FALSE;
   int32_t           return_code = SHELL_EXIT_SUCCESS;
   uint32_t          enet_device = BSP_DEFAULT_ENET_DEVICE;
   uint32_t          index = 1;
   _rtcs_if_handle   ihandle;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

    if (!print_usage)
    {
        if (argc > index)
        {
            if (Shell_parse_number (argv[index], &enet_device))
            {
                index++;
            }
        }

        if (enet_device >= BSP_ENET_DEVICE_COUNT)
        {
            printf ("Wrong number of ethernet device (%d)!\n", enet_device);
            return_code = SHELL_EXIT_ERROR;
            print_usage = TRUE;
        }
        else
        {
            if (argc > index)
            {
                return_code = SHELL_EXIT_ERROR;
                print_usage = TRUE;
            }
            else
            {
                ihandle = ipcfg_get_ihandle (enet_device);
                ARP_display_if_table(ihandle);
            }
        }
    }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s [<device>]\n", argv[0]);
      } else  {
         printf("Usage: %s  [<device>]\n", argv[0]);
         printf("   <device>      = Ethernet device number (default %d).\n", BSP_DEFAULT_ENET_DEVICE);
      }
   }

   return return_code;
} /* Endbody */

/*TASK*-----------------------------------------------------------------
*
* Function Name  : Shell_arpdel
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/

int32_t Shell_arpdel(int32_t argc, char *argv[] )
{ /* Body */
   bool              print_usage, shorthelp = FALSE;
   int32_t           return_code = SHELL_EXIT_SUCCESS;
   uint32_t          enet_device = BSP_DEFAULT_ENET_DEVICE;
   uint32_t          index = 1;
   _ip_address       ipaddr;
   _rtcs_if_handle   ihandle;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc > index)
      {
          if (Shell_parse_number (argv[index], &enet_device))
          {
              index++;
          }
      }

      if (enet_device >= BSP_ENET_DEVICE_COUNT)
      {
          printf ("Wrong number of ethernet device (%d)!\n", enet_device);
          return_code = SHELL_EXIT_ERROR;
          print_usage = TRUE;
      }
      else
      {
          if (argc != (index + 1)){
             return_code = SHELL_EXIT_ERROR;
             print_usage = TRUE;
          } else  if (!Shell_parse_ip_address(argv[index], &ipaddr))  {
              printf("Error, invalid ip address\n");
              return_code = SHELL_EXIT_ERROR;
          }
          if (return_code == SHELL_EXIT_SUCCESS) {
              ihandle = ipcfg_get_ihandle (enet_device);
              RTCS_arp_delete(ihandle, ipaddr);
          }
      }
   }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s [<device>] <ip_address>\n", argv[0]);
      } else  {
         printf("Usage: %s [<device>] <ip_address> \n", argv[0]);
         printf("   <device>      = Ethernet device number (default %d).\n", BSP_DEFAULT_ENET_DEVICE);
         printf("   <ip_address>  = IP Address\n");
      }
   }

   return return_code;
} /* Endbody */

/*TASK*-----------------------------------------------------------------
*
* Function Name  : Shell_arpadd
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/
int32_t Shell_arpadd(int32_t argc, char *argv[] )
{
    bool             print_usage, shorthelp = FALSE;
    int32_t          return_code = SHELL_EXIT_SUCCESS;
    uint32_t         enet_device = BSP_DEFAULT_ENET_DEVICE;
    uint32_t         index = 1;
    _enet_address    mac;
    _ip_address      ipaddr;
    _rtcs_if_handle  ihandle;

    print_usage = Shell_check_help_request(argc, argv, &shorthelp );

    if (!print_usage)
    {
        if (argc > index)
        {
            if (Shell_parse_number (argv[index], &enet_device))
            {
                index++;
            }
        }

        if (enet_device >= BSP_ENET_DEVICE_COUNT)
        {
            printf ("Wrong number of ethernet device (%d)!\n", enet_device);
            return_code = SHELL_EXIT_ERROR;
            print_usage = TRUE;
        }
        else
        {
            if ((argc != (index + 3))&&(argc != (index + 2)))
            {
                return_code = SHELL_EXIT_ERROR;
                print_usage = TRUE;
            }
            else  if (! Shell_parse_ip_address(argv[index], &ipaddr))
            {
                printf("Error, invalid ip address\n");
                return_code = SHELL_EXIT_ERROR;
            }
            else if (! Shell_parse_enet_address(argv[index + 1], mac))
            {
                printf("Invalid MAC address!\n");
                return_code = SHELL_EXIT_ERROR;
            }
            else
            {
                ihandle = ipcfg_get_ihandle (enet_device);
                RTCS_arp_add(ihandle, ipaddr, (char*)mac);
                ARP_display_if_table(ihandle);
            }
        }
    }


    if (print_usage)
    {
        if (shorthelp)
        {
            printf("%s [<device>] <ip_address> <mac>\n", argv[0]);
        }
        else
        {
            printf("Usage: %s [<device>] <ip_address> <mac>\n", argv[0]);
            printf("   <device>      = Ethernet device number (default %d).\n", BSP_DEFAULT_ENET_DEVICE);
            printf("   <ip_address>  = IP Address\n");
            printf("   <mac>         = Ethernet MAC address\n"  );
        }
    }

    return return_code;
}

#else /* (BSP_ENET_DEVICE_COUNT > 0) */
int32_t Shell_arpadd(int32_t argc, char *argv[] )
{
   printf ("Cannot use this command, no enet device driver available in this BSP.");
   return SHELL_EXIT_ERROR;
}
int32_t Shell_arpdel(int32_t argc, char *argv[] )
{
   printf ("Cannot use this command, no enet device driver available in this BSP.");
   return SHELL_EXIT_ERROR;
}
int32_t Shell_arpdisp(int32_t argc, char *argv[] )
{
   printf ("Cannot use this command, no enet device driver available in this BSP.");
   return SHELL_EXIT_ERROR;
}
#endif /* (BSP_ENET_DEVICE_COUNT > 0) */

#else /* BSP_ENET_DEVICE_COUNT */
int32_t Shell_arpadd(int32_t argc, char *argv[] )
{
   printf ("Cannot use this command, no enet device driver available in this BSP.");
   return SHELL_EXIT_ERROR;
}
int32_t Shell_arpdel(int32_t argc, char *argv[] )
{
   printf ("Cannot use this command, no enet device driver available in this BSP.");
   return SHELL_EXIT_ERROR;
}
int32_t Shell_arpdisp(int32_t argc, char *argv[] )
{
   printf ("Cannot use this command, no enet device driver available in this BSP.");
   return SHELL_EXIT_ERROR;
}
#endif /* BSP_ENET_DEVICE_COUNT */
#endif /* SHELLCFG_USES_RTCS */

/* EOF */

