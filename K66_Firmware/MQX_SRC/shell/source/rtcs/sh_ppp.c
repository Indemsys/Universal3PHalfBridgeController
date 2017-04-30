/*HEADER**********************************************************************
*
* Copyright 2012 Freescale Semiconductor, Inc.
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
*   This file contains the RTCS shell command related to PPP connection.
*
*
*END************************************************************************/
#include <ctype.h>
#include <string.h>
#include <mqx.h>
#include "shell.h"
#if SHELLCFG_USES_RTCS
#if RTCSCFG_ENABLE_PPP

#include <rtcs.h>
#include "sh_rtcs.h"
#include <ppp.h>
#include <fio.h>

static bool   PPP_link = FALSE;
static bool   PPP_pausing = FALSE;

SHELL_PPP_LINK ppp_conn;

extern PPP_SECRET_PTR _PPP_PAP_LSECRET;
extern PPP_SECRET_PTR _PPP_PAP_RSECRETS;
extern char       *_PPP_CHAP_LNAME;
extern PPP_SECRET_PTR _PPP_CHAP_LSECRETS;
extern PPP_SECRET_PTR _PPP_CHAP_RSECRETS;

void   sh_ppp_printhelp(int shorthelp);
void   sh_ppp_start(SHELL_PPP_LINK_PTR ppp_conn);
int32_t sh_ppp_pause(SHELL_PPP_LINK_PTR ppp_conn);
int32_t sh_ppp_resume(SHELL_PPP_LINK_PTR ppp_conn);
int32_t sh_ppp_stop(SHELL_PPP_LINK_PTR ppp_conn);

static int sh_ppp_handshake(char* device_name, bool listen_flag, unsigned int attempts);
static void PPP_linkup(void *lwsem);
static void PPP_linkdown(void *lwsem);
static void sh_ppp_task(void *params, void *creator);
static void sh_ppp_conn_task(void *params, void *creator);
static uint32_t sh_ppp_clone_string(char* src, char** dst_ptr);
/*FUNCTION*------------------------------------------------
*
* Function Name: Shell_ppp
* Comments     : Shell command "ppp" is starting or stopping PPP connection.
*
*END*-----------------------------------------------------*/
int32_t Shell_ppp(int32_t argc, char* argv[])
{
    uint32_t           i = 0;
    int               print_usage = 0;
    bool           shorthelp = FALSE;

    ppp_conn.LOCAL_NAME     = NULL;
    ppp_conn.LOCAL_PASSWORD = NULL;

    /* Check for help. */
    print_usage = Shell_check_help_request(argc, argv, &shorthelp);

    if (print_usage)
    {
        goto HELP;
    }
    /* Parse arguments */
    for (i = 1; i < argc; i++)
    {
        switch (i)
        {
            case (1):
                if(strcmp(argv[i], "stop") == 0)
                {
                    return(sh_ppp_stop(&ppp_conn));
                }
                else if(strcmp(argv[i], "connect") == 0)
                {
                    ppp_conn.listen_flag = 0;
                }
                else if(strcmp(argv[i], "listen") == 0)
                {
                    ppp_conn.listen_flag = 1;
                    /* In case of "server" we need have min 7 arg : ppp server ittyX: rlogin rpassword boardip clientip */
                    if(argc < 7)
                    {
                        print_usage = TRUE;
                    }
                }
                else if(strcmp(argv[i], "pause") == 0)
                {
                    if (ppp_conn.PPP_HANDLE == NULL)
                        return RTCS_OK;
                            
                    if (sh_ppp_pause(&ppp_conn) == RTCS_OK)
                    {
                        PPP_PRINTF(stderr, "PPP connection is paused\n");
                        return RTCS_OK;
                    }
                    return RTCS_ERROR;
                }
                else if(strcmp(argv[i], "resume") == 0)
                {
                    if (ppp_conn.PPP_HANDLE == NULL)
                        return RTCS_OK;
                    return sh_ppp_resume(&ppp_conn);
                }
                else
                {
                    print_usage = TRUE;
                }
                break;
            case (2):
                if(strstr(argv[i],"itty") != NULL)
                {
                    sh_ppp_clone_string(argv[i], &ppp_conn.device_name);
                }
                else
                {
                    PPP_PRINTF(stderr, "Invalid device name - \"%s\"", argv[i]);
                    print_usage = TRUE;
                }
                break;
            case (3):
                /* Save login */
                if(ppp_conn.listen_flag == 0)
                {
                    sh_ppp_clone_string(argv[i], &ppp_conn.LOCAL_NAME);
                }
                else
                {
                    sh_ppp_clone_string(argv[i], &ppp_conn.REMOTE_NAME);
                }
                break;
            case (4):
                /* Save password */
                if(ppp_conn.listen_flag==0)
                {
                    sh_ppp_clone_string(argv[i], &ppp_conn.LOCAL_PASSWORD);
                }
                else
                {
                    sh_ppp_clone_string(argv[i], &ppp_conn.REMOTE_PASSWORD);
                }
                break;
            case (5):
                /* Save local address */
                if(ppp_conn.listen_flag == 1)
                {
                    if(inet_pton(AF_INET, argv[i],&ppp_conn.PPP_LOCAL_ADDRESS,sizeof(ppp_conn.PPP_LOCAL_ADDRESS)) == (uint32_t)RTCS_ERROR)
                    {
                        print_usage = TRUE;
                    }
                }
                else
                {
                    print_usage = TRUE;
                }
                break;
            case (6):
                /* Save remote address */
                if(ppp_conn.listen_flag == 1)
                {
                    if(inet_pton(AF_INET, argv[i],&ppp_conn.PPP_REMOTE_ADDRESS,sizeof(ppp_conn.PPP_REMOTE_ADDRESS)) == (uint32_t)RTCS_ERROR)
                    {
                        print_usage = TRUE;
                    }
                }
                else
                {
                    print_usage = TRUE;
                }
                break;
            default:
                break;
        }
    }

HELP:
    if ((print_usage) || (argc < 2))
    {
        sh_ppp_printhelp(shorthelp);
        return(RTCS_OK);
    }

    /* Check if PPP is not already started. */
    if(ppp_conn.PPP_HANDLE)
    {
        PPP_PRINTF(stderr, "PPP already started. Stop it first.");
        return(RTCS_OK);
    }

    sh_ppp_start(&ppp_conn);
    /* shell_ppp_start() should create PPP_HANDLE */
    if(ppp_conn.PPP_HANDLE)
    {
        return RTCS_OK;
    }
    else
    {
        PPP_PRINTF(stdout, "The connection attemp failed because the modem (or other connecting device) on the remote computer is out of order.");
    }
    return(RTCS_OK);
}

/* Clone string using dynamic allocation */
static uint32_t sh_ppp_clone_string(char* src, char** dst_ptr)
{
    *dst_ptr = _mem_alloc_zero(strlen(src)+1);
    if (*dst_ptr != NULL)
    {
        strcpy(*dst_ptr, src);
    }
    else
    {
        return((uint32_t)RTCS_ERROR);
    }
    return(RTCS_OK);
}

/*FUNCTION*------------------------------------------------
*
* Function Name: shell_ppp_start()
* Comments     :
*   This function start PPP communication and try to establish PPP connection.
*END*-----------------------------------------------------*/
void sh_ppp_start(SHELL_PPP_LINK_PTR ppp_conn)
{
    uint32_t           error;
    PPP_SECRET        lsecret;
    PPP_PARAM_STRUCT  params;

    /* Set Async-Control-Character-Map to 0 */
    _PPP_ACCM = 0;
    /* Initialize secrets */
    _mem_zero(&lsecret, sizeof(lsecret));

    if(ppp_conn->listen_flag)
    {
        ppp_conn->PPP_GATE_ADDR = ppp_conn->PPP_LOCAL_ADDRESS;
        /* Fill  rsecrets[] array here. */
        ppp_conn->rsecrets[0].PPP_ID_LENGTH = strlen(ppp_conn->REMOTE_NAME);
        ppp_conn->rsecrets[0].PPP_PW_LENGTH = strlen(ppp_conn->REMOTE_PASSWORD);
        ppp_conn->rsecrets[0].PPP_ID_PTR    = ppp_conn->REMOTE_NAME;
        ppp_conn->rsecrets[0].PPP_PW_PTR    = ppp_conn->REMOTE_PASSWORD;
        ppp_conn->rsecrets[1].PPP_ID_LENGTH = 0;
        ppp_conn->rsecrets[1].PPP_PW_LENGTH = 0;
        ppp_conn->rsecrets[1].PPP_ID_PTR    = NULL;
        ppp_conn->rsecrets[1].PPP_PW_PTR    = NULL;
        _PPP_PAP_RSECRETS = ppp_conn->rsecrets;
    }
    else
    {
        lsecret.PPP_ID_LENGTH = strlen(ppp_conn->LOCAL_NAME);
        lsecret.PPP_PW_LENGTH = strlen(ppp_conn->LOCAL_PASSWORD);
        lsecret.PPP_ID_PTR    = ppp_conn->LOCAL_NAME;
        lsecret.PPP_PW_PTR    = ppp_conn->LOCAL_PASSWORD;
        _PPP_PAP_LSECRET      = &lsecret;
    }

    _lwsem_create(&ppp_conn->PPP_SEM, 1);

    _mem_zero(&params, sizeof(params));
    params.device = ppp_conn->device_name;
    params.up = PPP_linkup;
    params.down = PPP_linkdown;
    params.callback_param = &ppp_conn->PPP_SEM;
    params.local_addr = ppp_conn->PPP_LOCAL_ADDRESS;
    params.remote_addr = ppp_conn->PPP_REMOTE_ADDRESS;
    params.listen_flag = ppp_conn->listen_flag;

    ppp_conn->PPP_HANDLE = PPP_init(&params);
    if ( ppp_conn->PPP_HANDLE == NULL)
    {
       PPP_PRINTF(stderr,"Initialization failed. Error 0x%lx.", error);
       return;
    }

    PPP_PRINTF(stdout, "Initialized on '%s'.", ppp_conn->device_name);

    /* Start Windows OS handshake task */
    error = RTCS_task_create("PPP handshake", (RTCSCFG_DEFAULT_RTCSTASK_PRIO+1), 1500, sh_ppp_task, ppp_conn);
    if (error != RTCS_OK)
    {
       PPP_PRINTF(stderr,"Unable to set up handshake task. Error 0x%lx.", error);
       return;
    }
    
    error = RTCS_task_create("PPP connection", (RTCSCFG_DEFAULT_RTCSTASK_PRIO+2), 1500, sh_ppp_conn_task, ppp_conn);
    if (error != RTCS_OK)
    {
       PPP_PRINTF(stderr,"Unable to set up connection task. Error 0x%lx.", error);
       return;
    }

    if(ppp_conn->listen_flag == 0)
    {
        int count = 0;

        PPP_PRINTF(stdout,"Attempting to establish connection. Please wait...");
        /* 10 attempts to connect */
        for (count = 0; count<10 && !PPP_link; count++)
        {
            fprintf(stdout, ".");
            _time_delay(1000);
        }
        fprintf(stdout, "\n");

        if (PPP_link == TRUE)
        {
            _ip_address       local_address;
            _ip_address       remote_address = 0;

            local_address = IPCP_get_local_addr(params.if_handle);
            remote_address = IPCP_get_peer_addr(params.if_handle);
            PPP_PRINTF(stdout, "Connection established.");
            PPP_PRINTF(stdout, "Local address:  %d.%d.%d.%d", IPBYTES(local_address));
            PPP_PRINTF(stdout, "Remote address: %d.%d.%d.%d", IPBYTES(remote_address));
        }
        else
        {
            PPP_PRINTF(stderr,"Failed to connect to remote peer.");
            sh_ppp_stop(ppp_conn);
        }
    }
    /* Install a route for a default gateway */
    RTCS_gate_add(ppp_conn->PPP_GATE_ADDR, INADDR_ANY, INADDR_ANY);
}

/*FUNCTION*------------------------------------------------
*
* Function Name: shell_ppp_stop
* Comments     : Shell command "ppp stop" is calling this function to stop PPP connection.
*
*END*-----------------------------------------------------*/
int32_t sh_ppp_stop(SHELL_PPP_LINK_PTR ppp_conn)
{
    /* Stop PPP */
    if(ppp_conn->PPP_HANDLE)
    {
        uint32_t error = 0;
        
        /* Stop handshake and check physiscal connection tasks */
        _task_abort(ppp_conn->hstid);
        _task_abort(ppp_conn->conntid);
        
        error = PPP_release(ppp_conn->PPP_HANDLE);
        if (error != RTCS_OK)
        {
            PPP_PRINTF(stderr,"Failed to release connection. Error 0x%X", error);
        }
        ppp_conn->PPP_HANDLE = NULL;
    }
    /* Free memory */
    if (ppp_conn->device_name)
    {
        _mem_free(ppp_conn->device_name);
        ppp_conn->device_name = NULL;
    }
    if (ppp_conn->LOCAL_NAME)
    {
        _mem_free(ppp_conn->LOCAL_NAME);
        ppp_conn->LOCAL_NAME = NULL;
    }
    if (ppp_conn->REMOTE_NAME)
    {
        _mem_free(ppp_conn->REMOTE_NAME);
        ppp_conn->REMOTE_NAME = NULL;
    }
    if (ppp_conn->device_name)
    {
        _mem_free(ppp_conn->LOCAL_PASSWORD);
        ppp_conn->device_name = NULL;
    }
    if (ppp_conn->device_name)
    {
        _mem_free(ppp_conn->REMOTE_PASSWORD);
        ppp_conn->device_name = NULL;
    }
    /* Remove gate address */
    if(ppp_conn->listen_flag == 0)
    {
        RTCS_gate_remove(ppp_conn->PPP_GATE_ADDR, INADDR_ANY, INADDR_ANY);
    }

    _lwsem_destroy(&ppp_conn->PPP_SEM);
    PPP_PRINTF(stdout, "Connection released.");
    return(RTCS_OK);
}

/*FUNCTION*------------------------------------------------
*
* Function Name: sh_ppp_pause
* Comments     : Shell command "ppp pause" is calling this function to pause PPP connection.
*
*END*-----------------------------------------------------*/
int32_t sh_ppp_pause(SHELL_PPP_LINK_PTR ppp_conn)
{
    if(ppp_conn->PPP_HANDLE == NULL)
    {
        return RTCS_OK;
    }
    
    if (PPP_pausing == FALSE)
    {
        PPP_pausing = TRUE;
        if (PPP_pause(ppp_conn->PPP_HANDLE) == RTCS_OK)
        {
            return RTCS_OK;
        }
        PPP_pausing = FALSE;
        return RTCS_ERROR;
    }
    return RTCS_OK;
}

/*FUNCTION*------------------------------------------------
*
* Function Name: sh_ppp_resume
* Comments     : Shell command "ppp resume" is calling this function to resume PPP connection.
*
*END*-----------------------------------------------------*/
int32_t sh_ppp_resume(SHELL_PPP_LINK_PTR ppp_conn)
{
    if(ppp_conn->PPP_HANDLE == NULL)
    {
        return RTCS_OK;
    }
    
    if (PPP_pausing == TRUE)
    {
        PPP_PRINTF(stderr, "Resuming the connection...\n");
        return PPP_resume(ppp_conn->PPP_HANDLE);
    }
    return RTCS_OK;
}

void sh_ppp_printhelp(int shorthelp)
{
    if (shorthelp)
    {
        printf("ppp [param] [device name] [login] [password] [local IP] [remote IP]\n");
    }
    else
    {
        printf(" Usage: ppp\n");
        printf(" [param] [device name] [login] [password] [local IP] [remote IP]\n");
        printf(" [param]           - is \"listen\", \"connect\" or \"stop\"\n");
        printf(" [device name]     - is system name of interrupt driven communication device\n");
        printf("                     suitable for your board; i.e \"ittyX:\"\n");
        printf(" [login] and\n");
        printf(" [password]        - is \"login\" and \"password\" for PPP PAP.\n");
        printf("                     For \"listen\" login and pass should be defined. \n");
        printf("                     For \"listen\" and \"connect\" a device name is requered.\n");
        printf(" [local IP]  - is IP address of your board(listen mode).\n");
        printf(" [remote IP] - is IP address for client of your board(listen mode).\n");
        printf("\n Examples:\n");
        printf("    To connect to remote PPP host using \"ittyd:\" you need to type:\n");
        printf("        ppp connect ittyd: \n");
        printf("    or if you want to use password authentication (PAP):\n");
        printf("        ppp connect ittyd: login password \n");
        printf("\n    To start PPP with PAP in listening mode using \"ittyd:\":\n");
        printf("        ppp listen ittyd: login password brd_ip_addr remote_ip_addr\n");
        printf("        ppp listen ittyd: guest anon 192.168.0.1 192.168.0.217\n");
        printf("\n    To stop PPP connection you need type \"ppp stop\":\n");
        printf("        ppp stop\n");
        printf("\n    To pause PPP connection you need type \"ppp pause\":\n");
        printf("        ppp pause\n");
        printf("\n    To resume PPP connection you need type \"ppp resume\":\n");
        printf("        ppp resume\n\n");
    }
}

static void PPP_linkup(void *lwsem)
{
    PPP_link = TRUE;

    if (PPP_pausing == TRUE)
    {
        PPP_pausing = FALSE;
    }
    PPP_PRINTF(stdout, "Link Up");
}

static void PPP_linkdown(void *lwsem)
{
    PPP_link = FALSE;

    if (PPP_pausing == FALSE)
    {
        /* If link is down by calling "pause" function then don't need handshake again */
        _lwsem_post(lwsem);
    }
    PPP_PRINTF(stdout, "Link Down");
}

/* Task for checking if cable is disconnected or not*/
static void sh_ppp_conn_task(void *params, void *creator)
{
    ppp_conn.conntid = _task_get_id();
    
    RTCS_task_resume_creator(creator, RTCS_OK);
    
    while (1)
    {
        _time_delay(10000);
        if (PPP_link == TRUE)
        {
            if (PPP_check_connection(ppp_conn.PPP_HANDLE) == PPP_DISCONNECTED)
            {
                PPP_PRINTF(stderr, "Cable is disconnected");
                if (ppp_conn.PPP_HANDLE)
                {
                    PPP_pause(ppp_conn.PPP_HANDLE);
                }
            }
        }
    }
}

/* Task for handling the PPP handshake with Windows PC */
static void sh_ppp_task(void *params, void *creator)
{
    ppp_conn.hstid = _task_get_id();

    RTCS_task_resume_creator(creator, RTCS_OK);

    while(1)
    {
        /* Wait until PPP link is down */
        _lwsem_wait(&ppp_conn.PPP_SEM);

        /* If PPP is started, pause it */
        if (ppp_conn.PPP_HANDLE)
        {
            PPP_pause(ppp_conn.PPP_HANDLE);
        }
        /*
        ** Do the handshake:
        **  - If we are connecting try 10 times before giving up. Every attemp takes 2s -> wait for 20s.
        **  - If we are listening wait indefinitely.
        */
        sh_ppp_handshake(ppp_conn.device_name, ppp_conn.listen_flag, 10);
        /* Wait some time so WinPC wakes up */
        _time_delay(250);
        /* Resume PPP */
        if (ppp_conn.PPP_HANDLE)
        {
            PPP_resume(ppp_conn.PPP_HANDLE);
        }
        /*
        ** Wait 15 seconds for PPP link to go up.
        ** This allows enough time for PPP connection negotiation.
        */
        _time_delay(15000);
        if (PPP_link == FALSE && PPP_pausing == FALSE)
        {
            /* If the link is still down try handshake again */
            _lwsem_post(&ppp_conn.PPP_SEM);
        }
    }
}

/* Handshake function - blocking for listen mode, timeout for connect mode */
static int sh_ppp_handshake(char* device_name, bool listen_flag, unsigned int attempts)
{
    int count = 0;
    char c;
    char* match_ptr = NULL;
    int length = 0;
    char* send = NULL;
    char* recv = NULL;
    int retval = 0;
    MQX_FILE_PTR device_h;

    device_h = fopen(device_name, NULL);
    if (device_h == NULL)
    {
        return(1);
    }

    if (listen_flag)
    {
        PPP_PRINTF(stdout, "Waiting for incoming connection...");
        send = "CLIENTSERVER";
        recv = "CLIENTCLIENT";
    }
    else
    {
        send = "CLIENTCLIENT";
        recv = "CLIENTSERVER";
    }

    length = strlen(recv);
    match_ptr = recv;

    /* Wait for client and then send response */
    if (listen_flag)
    {
        for (;;)
        {
            c = fgetc(device_h);
            count++;

            if (c != *match_ptr++)
            {
                match_ptr = recv;
                count = 0;
            }

            if (count == length)
            {
                break;
            }
        }
        fwrite(send, 1, strlen(send), device_h);
        fflush(device_h);
    }
    else /* Send client string and wait for response */
    {
        int i;

        for (i = 0; i < attempts; i++)
        {
            int fails = 0;

            fwrite(send, 1, strlen(send), device_h);
            fflush(device_h);

            /*
            ** Wait for remote peer response for 2 seconds (20*100ms) then try again.
            ** Two seconds seems like good compromise between speed and reliability.
            */
            while(fails < 20)
            {
                bool param;
                /* Check if there is character available for reading */
                ioctl(device_h, IO_IOCTL_CHAR_AVAIL, &param);
                if (param)
                {
                    c = fgetc(device_h);
                    count++;

                    if (c != *match_ptr++)
                    {
                        match_ptr = recv;
                        count = 0;
                    }

                    if (count == length)
                    {
                        break;
                    }
                }
                else
                {
                    fails++;
                    _time_delay(100);
                }
            }
            if (fails < 20)
            {
                break;
            }
        }
        if (i >= attempts)
        {
            retval = 1;
        }
    }
    fclose(device_h);
    return(retval);
}
#endif /* RTCSCFG_ENABLE_PPP */
#endif /* SHELLCFG_USES_RTCS */
