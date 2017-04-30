#ifndef __rtcs_hosts_h__
#define __rtcs_hosts_h__
/*HEADER**********************************************************************
 *
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * Freescale Confidential and Proprietary - use of this software is
 * governed by the Freescale MQX RTOS License distributed with this
 * material. See the MQX_RTOS_LICENSE file distributed for more
 * details.
 *
 *****************************************************************************
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
 *****************************************************************************
 *
 * Comments:
 *
 *   RTCS Hosts file.
 *
 *END************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* http://en.wikipedia.org/wiki/Hosts_(file):
* The hosts file is a computer file used by an operating system to map hostnames to IP addresses.
* The hosts file is one of several system facilities that assists in addressing network nodes in 
* a computer network. It is a common part of an operating system's Internet Protocol (IP) implementation,
* and serves the function of translating human-friendly hostnames into numeric protocol addresses, 
* called IP addresses, that identify and locate a host in an IP network. */
void *RTCS_hosts_get_addr(const char *host_name, int family);
const char *RTCS_hosts_get_name(const sockaddr  *sa);

#ifdef __cplusplus
}
#endif

#endif /* __rtcs_hosts_h__ */


