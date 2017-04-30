/**HEADER********************************************************************
* 
* Copyright (c) 2013 Freescale Semiconductor;
* All Rights Reserved                       
*
*************************************************************************** 
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* $FileName: ftpsrv.c$
*
* Comments:
*
*   This file contains the RTCS shell address resolution command.
*
*END************************************************************************/
#include <ctype.h>
#include <string.h>
#include <mqx.h>
#include "shell.h"

#if SHELLCFG_USES_RTCS

#include <rtcs.h>
#include "sh_rtcs.h"


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_getname
*  Returned Value:  none
*  Comments  :  SHELL utility to resolve an address.
*
*END*-----------------------------------------------------------------*/
int32_t  Shell_getname(int32_t argc, char *argv[])
{
    bool            print_usage, shorthelp = FALSE;
    int32_t         return_code = SHELL_EXIT_SUCCESS;

    print_usage = Shell_check_help_request(argc, argv, &shorthelp );

    if (!print_usage)
    {
        if (argc == 2)
        {
            struct addrinfo     addrinfo_hints;
            struct addrinfo     *addrinfo_result;
            char                host_name[NI_MAXHOST];

            _mem_zero(&addrinfo_hints, sizeof(addrinfo_hints));
            addrinfo_hints.ai_flags = AI_NUMERICHOST;

            if (getaddrinfo(argv[1], NULL, &addrinfo_hints, &addrinfo_result) == 0) /* String to sockaddr.*/
            {
                if(addrinfo_result)
                {
                    /* Resolve address.*/
                    if (getnameinfo(addrinfo_result->ai_addr,
                                        addrinfo_result->ai_addrlen, 
                                        host_name, NI_MAXHOST,
                                        NULL, 0, 0) == 0) 
                    {
                        printf("%s\n", host_name);
                    }
                    else
                    {
                        printf("Failed to resolve: %s.\n", argv[1]);
                    }
                     
                    freeaddrinfo(addrinfo_result);
                }
            }
            else
            {
                printf("Wrong IP address: %s.\n", argv[1]);
                return_code = SHELL_EXIT_ERROR;
            }
        }
        else
        {
            printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
            return_code = SHELL_EXIT_ERROR;
            print_usage = TRUE;
        }
        
   }
   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <ip>\n", argv[0]);
      } else  {
         printf("Usage: %s <ip>\n", argv[0]);
         printf("\t<ip>    = IP address to use, both families. \n");
      }
   }
   return return_code;
}

#endif /* SHELLCFG_USES_RTCS */

