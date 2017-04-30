/**HEADER********************************************************************
*
* Copyright (c) 2013- 2014 Freescale Semiconductor;
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
* $FileName: osadapter_mqx.c$
* $Version :
* $Date    :
*
* Comments:
*
* @brief  The file includes the implementation of MQX of OS adapter.
*****************************************************************************/

#include "adapter_cfg.h"
#include "adapter_types.h"


#if (OS_ADAPTER_ACTIVE_OS == OS_ADAPTER_MQX)
  #include "mqx.h"
  #include "bsp.h"
  #include "lwevent.h"
  #include "lwmsgq.h"
  #include "lwgpio.h"
  #include "mutex.h"
  #include "lwsem.h"
  #if (defined(CPU_MK22F51212))
    #include "MK22F51212.h"
  #elif (defined(CPU_MK70F12))
    #include "MK70F12.h"
  #elif (defined(CPU_MK60FN1M012))
    #include "MK60F12.h"
  #elif (defined(CPU_VF65GS10_M4) || defined(CPU_VF65GS10_A5))
    #include "MVF61NS151MK50.h"
  #elif (defined(CPU_SVF522R3K_M4) || defined(CPU_SVF522R3K_A5))
    #include "SVF522R3MK4.h"
  #elif (defined(CPU_MK60D100M))
    #include "MK60D10.h"
  #elif (defined(CPU_MK21D5))
    #include "MK21D5.h"
  #elif (defined(CPU_MK21F12))
    #include "MK21F12.h"
  #elif (defined(CPU_MK60N512VMD100))
    #include "MK60DZ10.h"
  #endif
  #include "usb_misc.h"

uint32_t OS_Task_create(task_start_t pstart, void *param, uint32_t pri, uint32_t stack_size, char *task_name, void *opt)
{
  _task_id task_id;
  TASK_TEMPLATE_STRUCT task_template;

  task_template.TASK_TEMPLATE_INDEX = 0;
  task_template.TASK_ADDRESS = (TASK_FPTR)pstart;
  task_template.TASK_STACKSIZE = stack_size;
  task_template.TASK_PRIORITY = pri;
  task_template.TASK_NAME = task_name;
  if (opt != NULL)
  {
    task_template.TASK_ATTRIBUTES = *((uint32_t *)opt);
  }
  else
  {
    task_template.TASK_ATTRIBUTES = 0;
  }

  task_template.CREATION_PARAMETER = (uint32_t)param;
  task_template.DEFAULT_TIME_SLICE = 0;

  task_id = _task_create_blocked(0, 0, (uint32_t)&task_template);

  if (task_id == MQX_NULL_TASK_ID)
  {
    return (uint32_t)OS_TASK_ERROR;
  }

  _task_ready(_task_get_td(task_id));
  return (uint32_t)task_id;
}

uint32_t OS_Task_delete(uint32_t task_id)
{
  uint32_t ret;
  ret = _task_destroy((_task_id)task_id);
  if (ret != MQX_OK) return (uint32_t)OS_TASK_ERROR;
  else return (uint32_t)OS_TASK_OK;
}

uint32_t OS_Task_suspend(uint32_t task_id)
{
  task_id = task_id;
  _task_block();
  return (uint32_t)OS_TASK_OK;
}

uint32_t OS_Task_resume(uint32_t task_id)
{
  _task_ready(_task_get_td(task_id));
  return (uint32_t)OS_TASK_OK;
}

os_event_handle OS_Event_create(uint32_t flag)
{
  LWEVENT_STRUCT *event;
  event = (LWEVENT_STRUCT *)_mem_alloc_system_zero(sizeof(LWEVENT_STRUCT));
  if (event == NULL)
  {
    return NULL;
  }

  if (_lwevent_create(event, flag) != MQX_OK)
  {
    _mem_free((void *)event);
    return NULL;
  }
  return (os_event_handle)event;
}

uint32_t OS_Event_destroy(os_event_handle handle)
{
  LWEVENT_STRUCT *event = (LWEVENT_STRUCT *)handle;
  if (_lwevent_destroy(event) != MQX_OK)
  {
    _mem_free((void *)event);
    return (uint32_t)OS_EVENT_ERROR;
  }
  _mem_free((void *)event);
  return OS_EVENT_OK;
}

uint32_t OS_Event_check_bit(os_event_handle handle, uint32_t bitmask)
{
  LWEVENT_STRUCT *event = (LWEVENT_STRUCT *)handle;
  if ((event->VALUE & bitmask) != 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
uint32_t OS_Event_get_value(os_event_handle handle)
{
  LWEVENT_STRUCT *event = (LWEVENT_STRUCT *)handle;
  return event->VALUE;
}
uint32_t OS_Event_clear(os_event_handle handle, uint32_t bitmask)
{
  LWEVENT_STRUCT *event = (LWEVENT_STRUCT *)handle;
  if (_lwevent_clear(event, bitmask) != MQX_OK)
  {
    return (uint32_t)OS_EVENT_ERROR;
  }
  return (uint32_t)OS_EVENT_OK;
}


uint32_t OS_Event_set(os_event_handle handle, uint32_t bitmask)
{
  LWEVENT_STRUCT *event = (LWEVENT_STRUCT *)handle;
  if (_lwevent_set(event, bitmask) != MQX_OK)
  {
    return (uint32_t)OS_EVENT_ERROR;
  }
  return (uint32_t)OS_EVENT_OK;
}

uint32_t OS_Event_wait(os_event_handle handle, uint32_t bitmask, uint32_t flag, uint32_t timeout)
{
  LWEVENT_STRUCT *event = (LWEVENT_STRUCT *)handle;
  uint32_t ret;

  ret = _lwevent_wait_ticks(event, bitmask, flag, timeout * _time_get_ticks_per_sec() / 1000);
  //printf("os 0x%x\n", ret);
  if (LWEVENT_WAIT_TIMEOUT == ret)
  {
    return (uint32_t)OS_EVENT_TIMEOUT;
  }
  else if (MQX_OK == ret)
  {
    return (uint32_t)OS_EVENT_OK;
  }
  return (uint32_t)OS_EVENT_ERROR;
}

os_msgq_handle OS_MsgQ_create(uint32_t max_msg_number, uint32_t msg_size)
{
  void *msgq;
  uint32_t size = sizeof(LWMSGQ_STRUCT) + max_msg_number * msg_size * 4;

  msgq = _mem_alloc_system_zero(size);
  if (msgq == NULL)
  {
    return NULL;
  }

  if (_lwmsgq_init(msgq, max_msg_number, msg_size) != MQX_OK)
  {
    _mem_free(msgq);
    return NULL;
  }

  return (os_msgq_handle)msgq;
}

uint32_t OS_MsgQ_send(os_msgq_handle msgq, void *msg, uint32_t flag)
{
  if (MQX_OK != _lwmsgq_send(msgq, (uint32_t *)msg, flag))
  {
    return (uint32_t)OS_MSGQ_ERROR;
  }
  return (uint32_t)OS_MSGQ_OK;
}

uint32_t OS_MsgQ_recv(os_msgq_handle msgq, void *msg, uint32_t flag, uint32_t timeout)
{
  if (MQX_OK != _lwmsgq_receive(msgq, (uint32_t *)msg, flag, timeout, NULL))
  {
    return (uint32_t)OS_MSGQ_ERROR;
  }
  return (uint32_t)OS_MSGQ_OK;
}

uint32_t OS_MsgQ_Is_Empty(os_msgq_handle msgq, void *msg)
{
  uint32_t ret;
  ret = LWMSGQ_IS_EMPTY(msgq);
  if (!ret)
  {
    if (MQX_OK != _lwmsgq_receive(msgq, (uint32_t *)msg, OS_MSGQ_RECEIVE_BLOCK_ON_EMPTY, 1, NULL))
    {
      return (uint32_t)OS_MSGQ_ERROR;
    }
  }
  return ret;

}

uint32_t OS_MsgQ_destroy(os_msgq_handle msgq)
{
  _lwmsgq_deinit(msgq);
  _mem_free(msgq);
  return (uint32_t)OS_MSGQ_OK;
}

os_gpio_handle OS_Gpio_init(uint32_t id, uint32_t dir, uint32_t value)
{
  LWGPIO_STRUCT_PTR          pGpio;
  pGpio = (LWGPIO_STRUCT_PTR)_mem_alloc_system_zero(sizeof(LWGPIO_STRUCT));
  if (pGpio == NULL)
  {
    return NULL;
  }

  if (lwgpio_init(pGpio, (LWGPIO_PIN_ID)id, (LWGPIO_DIR)dir, (LWGPIO_VALUE)value))
  {
    return (os_gpio_handle)pGpio;
  }
  else
  {
    _mem_free((void *)pGpio);
    return NULL;
  }
}

uint32_t OS_Gpio_set_functionality(os_gpio_handle handle, uint32_t function)
{
  LWGPIO_STRUCT_PTR    pGpio = (LWGPIO_STRUCT_PTR)handle;
  lwgpio_set_functionality(pGpio, function);
  return OS_GPIO_OK;
}

uint32_t OS_Gpio_set_value(os_gpio_handle handle, uint32_t value)
{
  LWGPIO_STRUCT_PTR    pGpio = (LWGPIO_STRUCT_PTR)handle;
  lwgpio_set_value(pGpio, (LWGPIO_VALUE)value);
  return OS_GPIO_OK;
}

uint32_t OS_Gpio_deinit(os_gpio_handle handle)
{
  _mem_free((void *)handle);
  return OS_GPIO_OK;
}

os_mutex_handle OS_Mutex_create()
{
  MUTEX_STRUCT_PTR mutex = NULL;
  mutex = _mem_alloc_system_zero(sizeof(MUTEX_STRUCT));
  if (mutex == NULL)
  {
    return NULL;
  }
  if (_mutex_init(mutex, NULL) != MQX_OK)
  {
    _mem_free(mutex);
    return NULL;
  }
  return (os_mutex_handle)mutex;
}

uint32_t OS_Mutex_lock(os_mutex_handle mutex)
{
  if (_mutex_lock((MUTEX_STRUCT_PTR)mutex) != MQX_OK)
  {
    return (uint32_t)OS_MUTEX_ERROR;
  }
  else
  {
    return (uint32_t)OS_MUTEX_OK;
  }
}

uint32_t OS_Mutex_unlock(os_mutex_handle mutex)
{
  if (_mutex_unlock((MUTEX_STRUCT_PTR)mutex) != MQX_OK)
  {
    return (uint32_t)OS_MUTEX_ERROR;
  }
  else
  {
    return (uint32_t)OS_MUTEX_OK;
  }
}

uint32_t OS_Mutex_destroy(os_mutex_handle mutex)
{
  _mutex_destroy((MUTEX_STRUCT_PTR)mutex);
  _mem_free(mutex);
  return OS_MUTEX_OK;
}

os_sem_handle OS_Sem_create(uint32_t initial_number)
{
  LWSEM_STRUCT_PTR sem = NULL;
  sem = (LWSEM_STRUCT_PTR)_mem_alloc_system_zero(sizeof(LWSEM_STRUCT));
  if (sem == NULL)
  {
    return NULL;
  }
  if (_lwsem_create(sem, initial_number) != MQX_OK)
  {
    _mem_free(sem);
    return NULL;
  }
  return (os_sem_handle)sem;
}

uint32_t OS_Sem_wait(os_sem_handle sem, uint32_t timeout)
{
  uint32_t result = _lwsem_wait_ticks((LWSEM_STRUCT_PTR)sem, timeout);
  if (result == MQX_LWSEM_WAIT_TIMEOUT)
  {
    return (uint32_t)OS_SEM_TIMEOUT;
  }
  else if (result == MQX_OK)
  {
    return (uint32_t)OS_SEM_OK;
  }
  else
  {
    return (uint32_t)OS_SEM_ERROR;
  }
}

uint32_t OS_Sem_post(os_sem_handle sem)
{
  uint32_t result = _lwsem_post((LWSEM_STRUCT_PTR)sem);
  if (result == MQX_OK)
  {
    return (uint32_t)OS_SEM_OK;
  }
  else
  {
    return (uint32_t)OS_SEM_ERROR;
  }
}


uint32_t OS_Sem_destroy(os_sem_handle sem)
{
  uint32_t result = _lwsem_destroy((LWSEM_STRUCT_PTR)sem);
  _mem_free(sem);
  if (result == MQX_OK)
  {
    return (uint32_t)OS_SEM_OK;
  }
  else
  {
    return (uint32_t)OS_SEM_ERROR;
  }
}

void OS_Time_delay(register uint32_t milliseconds)
{
  _time_delay(milliseconds);
  return;
}

uint32_t soc_get_usb_base_address(uint8_t controller_id)
{
  if (controller_id == USB_CONTROLLER_KHCI_0)
  {
    return (uint32_t)USB0_BASE_PTR;
  }
  else if (controller_id == USB_CONTROLLER_EHCI_0)
  {
  #if (defined(CPU_MK70F12)) || (defined(CPU_MK65F18) || defined(CPU_MK60FN1M012))
    return (uint32_t)USBHS_BASE_PTR;
  #elif (defined(CPU_VF65GS10_M4) || defined(CPU_VF65GS10_A5))
    return (uint32_t)USB0_BASE_PTR;
  #elif (defined(CPU_SVF522R3K_M4) || defined(CPU_SVF522R3K_A5))
    return (uint32_t)USB0_BASE_PTR;
  #endif
  }
  else if (controller_id == USB_CONTROLLER_EHCI_1)
  {
  #if (defined(CPU_VF65GS10_M4) || defined(CPU_VF65GS10_A5))
    return (uint32_t)USB1_BASE_PTR;
  #elif (defined(CPU_SVF522R3K_M4) || defined(CPU_SVF522R3K_A5))
    return (uint32_t)USB1_BASE_PTR;
  #endif
  }
  else
  {
    return (uint32_t)NULL;
  }
  return (uint32_t)NULL;
}

uint8_t soc_get_usb_vector_number(uint8_t controller_id)
{
  if (controller_id == USB_CONTROLLER_KHCI_0)
  {
  #if !(defined(CPU_VF65GS10_A5) || defined(CPU_VF65GS10_M4) || \
      defined(CPU_SVF522R3K_M4) || defined(CPU_SVF522R3K_A5))
    return INT_USB0;
  #else
    return 0;
  #endif
  }
  else if (controller_id == USB_CONTROLLER_EHCI_0)
  {
  #if (defined(CPU_VF65GS10_A5) || defined(CPU_SVF522R3K_A5))
    return GIC_USB0;
  #elif (defined(CPU_VF65GS10_M4) || defined(CPU_SVF522R3K_M4))
    return NVIC_USB0;
  #elif (defined(CPU_MK70F12) || defined(CPU_MK65F18) || defined(CPU_MK60FN1M012))
    return INT_USBHS;
  #endif
  }
  else if (controller_id == USB_CONTROLLER_EHCI_1)
  {
  #if (defined(CPU_VF65GS10_A5) || defined(CPU_SVF522R3K_A5))
    return GIC_USB1;
  #elif (defined(CPU_VF65GS10_M4) || defined(CPU_SVF522R3K_M4))
    return NVIC_USB1;
  #endif
  }
  else
  {
    return 0;
  }
  return 0;
}

  #if 0
static inline void OS_Lock(void)
{
  _int_disable();
}

void OS_Unlock(void)
{
  _int_enable();
}
  #endif

void* OS_Mem_alloc(uint32_t size)
{
  return _mem_alloc_system(size);
}

void* OS_Mem_alloc_zero(uint32_t size)
{
  return _mem_alloc_system_zero(size);
}

void* OS_Mem_alloc_uncached(uint32_t size)
{
  #if PSP_HAS_DATA_CACHE
  return _mem_alloc_system_uncached(size);
  #else
  return _mem_alloc_system(size);
  #endif
}

void* OS_Mem_alloc_uncached_zero(uint32_t size)
{
  #if PSP_HAS_DATA_CACHE
  return _mem_alloc_system_zero_uncached(size);
  #else
  return _mem_alloc_system_zero(size);
  #endif
}

uint32_t OS_Mem_free(void *ptr)
{
  return _mem_free(ptr);
}

void OS_Mem_copy(void *src, void *dst, uint32_t length)
{
  #if MEM_COPY_ENHANCEMENT
  _mem_copy(src, dst, length);
  #else
  memcpy(dst, src, length);
  #endif
}

void OS_Mem_zero(void *ptr, uint32_t length)
{
  #if MEM_ZERO_ENHANCEMENT
  _mem_zero(ptr, length);
  #else
  memset(ptr, 0, length);
  #endif
}

osa_int_isr_fptr OS_install_isr(uint32_t vector, osa_int_isr_fptr isr, void *data)
{
  return _int_install_isr(vector, isr, data);
}

void OS_dcache_invalidate_mlines(void *addr, uint32_t length)
{
  #if (PSP_HAS_DATA_CACHE)
  if ((addr < (void *)__UNCACHED_DATA_START) || (((uint32_t)addr + length) > (uint32_t)__UNCACHED_DATA_END))
  {
    if (((uint32_t)addr & (PSP_CACHE_LINE_SIZE - 1)) != 0)
    {
      _dcache_flush_mlines((void *)addr, PSP_CACHE_LINE_SIZE);
    }
    if (((uint32_t)((uint8_t *)addr + length) & (PSP_CACHE_LINE_SIZE - 1)) != 0)
    {
      _dcache_flush_mlines((uint8_t *)addr + length, PSP_CACHE_LINE_SIZE);
    }
    _dcache_invalidate_mlines(addr, length);
  }
  #endif
}

void OS_dcache_flush_mlines(void *addr, uint32_t length)
{
  #if (PSP_HAS_DATA_CACHE)
  if ((addr < (void *)__UNCACHED_DATA_START) || (((uint32_t)addr + length) > (uint32_t)__UNCACHED_DATA_END))
  {
    _dcache_flush_mlines(addr, length);
  }
  #endif
}


void* OS_Mem_alloc_uncached_align(uint32_t n, uint32_t m)
{
  void *p = NULL;
  #if PSP_HAS_DATA_CACHE
  p = _mem_alloc_align_uncached(n, m);
  _mem_transfer(p, _task_get_id(), _mqx_get_system_task_id());
  #else
  p = _mem_alloc_system_align(n, m);
  #endif
  return p;
}

#endif


