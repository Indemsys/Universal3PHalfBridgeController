/**HEADER********************************************************************
*
* Copyright (c) 2013 - 2014 Freescale Semiconductor;
* All Rights Reserved
*
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
* $FileName: osadapter_mqx.h$
* $Version :
* $Date    :
*
* Comments:
*
* @brief The file contains the definition of BM of OS adapter header.
*
*****************************************************************************/

#ifndef _USB_OSADAPTER_MQX_H
  #define _USB_OSADAPTER_MQX_H 1

  #include "mqx.h"
  #include "bsp.h"
  #include "lwevent.h"
  #include "lwmsgq.h"
  #include "lwgpio.h"
  #include "mutex.h"
  #include "lwsem.h"
  #if 0
extern void _int_disable(void);
extern void _int_enable(void);
extern int32_t _io_printf(const char  *fmt_ptr, ...);
extern void * _mem_alloc_system(uint32_t size);
extern void * _mem_alloc_system_uncached(uint32_t size);
extern void * _mem_alloc_system_zero_uncached(uint32_t size);
extern void * _mem_alloc_system_zero(uint32_t size);
extern uint32_t _mem_free(void*);

extern osa_int_isr_fptr _int_install_isr(uint32_t, osa_int_isr_fptr, void *);
extern uint32_t _nvic_int_init(uint32_t, uint32_t, bool);
extern void _dcache_invalidate_mlines(void *addr, uint32_t length);
extern void _dcache_flush_mlines(void *addr, uint32_t length);
extern void _time_delay(register uint32_t milliseconds);
extern uint32_t _time_get_ticks_per_sec(void);
  #endif
extern uint8_t soc_get_usb_vector_number(uint8_t controller_id);
extern uint32_t soc_get_usb_base_address(uint8_t controller_id);
extern void OS_dcache_invalidate_mlines(void *addr, uint32_t length);
extern void OS_dcache_flush_mlines(void *addr, uint32_t length);
extern void OS_Time_delay(register uint32_t milliseconds);
extern uint32_t OS_Event_get_value(os_event_handle handle);

  #if PSP_ENDIAN == MQX_BIG_ENDIAN
    #define ENDIANNESS           0
  #else
    #define ENDIANNESS           1
  #endif


  #define  USB_PRINTF                         RTT_terminal_printf// _io_printf

  #define  TICKS_PER_SEC                      _time_get_ticks_per_sec()

/* memory operation wrapper */
  #define OS_Lock()                           _int_disable()
  #define OS_Unlock()                         _int_enable()
  #define OS_install_isr(vector, isr, data)   _int_install_isr(vector, isr, data)
  #define OS_intr_init(num, prior, subprior, enable)     _bsp_int_init(num, prior, subprior, enable)

  #if PSP_HAS_DATA_CACHE
    #define OS_Mem_alloc_uncached(n)         _mem_alloc_system_uncached(n)
    #define OS_Mem_alloc_uncached_zero(n)    _mem_alloc_system_zero_uncached(n)
  #else
    #define OS_Mem_alloc_uncached(n)         _mem_alloc_system(n)
    #define OS_Mem_alloc_uncached_zero(n)    _mem_alloc_system_zero(n)
  #endif // PSP_HAS_DATA_CACHE
extern void* OS_Mem_alloc_uncached_align(uint32_t, uint32_t);

  #define OS_Mem_alloc(n)                      _mem_alloc_system(n)
  #define OS_Mem_alloc_zero(n)                 _mem_alloc_system_zero(n)
  #define OS_Mem_free(ptr)                     _mem_free(ptr)
  #define OS_Mem_zero(ptr,n)                   _mem_zero(ptr,n)
  #define OS_Mem_copy(src,dst,n)               _mem_copy(src,dst,n)


/****************************************************************************
 ****************************************************************************/
//#define OS_Event_set                         _lwevent_set
//#define OS_Event_clear                       _lwevent_clear
//#define OS_Time_delay                        _time_delay
//#define OS_Sem_post                          _lwsem_post

  #if MEM_COPY_ENHANCEMENT
extern void             _mem_copy(void *, void *, uint32_t);
  #else
    #include <string.h>
    #define _mem_copy(s,d,l) memcpy(d,s,l)
  #endif

  #if MEM_ZERO_ENHANCEMENT
extern void             _mem_zero(void *, uint32_t);
  #else
    #include <string.h>
    #define _mem_zero(p,n)  memset(p,0,n)
  #endif
#endif


