#include "App.h"


volatile CAN_MemMapPtr           CAN_PTR;
LWEVENT_STRUCT                   can_tx_event;
LWEVENT_STRUCT                   can_rx_event;
uint32_t                         can_events_mask;
T_can_statistic                  can_stat;
_queue_id                        can_tx_queue_id;
_pool_id                         can_msg_pool_id;

#ifdef ENABLE_CAN_LOG
uint32_t                         canrx_log_head;
uint32_t                         canrx_log_tail;
T_can_rx                         canrx_log[CAN_RX_LOG_SZ];
LWSEM_STRUCT                     can_log_sem;
#endif

const T_can_rx_config         rx_mboxes[RX_MBOX_CNT] =
{
  // mb_num      canid                    canid_mask    broadcast
  { CAN_RX_MB1, ALL_ENTER_TO_BOOT_MOD,    0x1FFFFFFF ,   1          }, // В маске биты со значением 1 соответствуют проверяемым битам идентификатора, со значением 0 - не проверяемым  
  { CAN_RX_MB2, DMC01_REQ,                0x1FFFFFFF ,   0          }, // 
  { CAN_RX_MB3, DMC01_REQ,                0x1FFFFFFF ,   0          }, // 
  { CAN_RX_MB4, DMC01_REQ,                0x1FFFFFFF ,   0          }, // 
  { CAN_RX_MB5, DMC01_REQ,                0x1FFFFFFF ,   0          }, // 
  { CAN_RX_MB6, DMC01_REQ,                0x1FFFFFFF ,   0          }, // 
  { CAN_RX_MB7, DMC01_REQ,                0x1FFFFFFF ,   0          }, // 
  { CAN_RX_MB8, DMC01_REQ,                0x1FFFFFFF ,   0          }, // 
};



/*-------------------------------------------------------------------------------------------------------------
  Обслуживание прерываний CAN шины при отправке и получении данных
-------------------------------------------------------------------------------------------------------------*/
void CAN_isr(void *ptr)
{
  volatile CAN_MemMapPtr CAN;
  uint32_t           reg;

  CAN = (CAN_MemMapPtr)ptr;

  reg = CAN->IFLAG1; // Получить флаги прерываний (всего в работе 16-ть младших флагов).
                     // Если не разрешен FIFO то каждый флаг отмечает прерывание от соответствующего буфера.
                     // Снимаються флаги записью 1
                     // Каждый флаг может отмечать, как прерывание по приему так и по передаче.
  if (reg & 0x0FF)
  {
    _lwevent_set(&can_rx_event, reg & 0x0FF); // Создать для каждого флага событие
  }

  if (reg & BIT(CAN_TX_MB1))
  {
    _lwevent_set(&can_tx_event, BIT(CAN_TX_MB1)); // Создать для каждого флага событие
  }
  CAN->IFLAG1 = reg;                              // Сбрасываем флаги, но только те о которых мы сообщили
}
/*-------------------------------------------------------------------------------------------------------------
  Обслуживание прерываний CAN шины в результате ошибок м других событий
-------------------------------------------------------------------------------------------------------------*/
void CAN_err_isr(void *ptr)
{
  volatile CAN_MemMapPtr CAN;
  uint32_t           reg;

  CAN = (CAN_MemMapPtr)ptr;

  // Получить флаги ошибок и сбросить их
  reg = CAN->ESR1 & (
                     BIT(17)   // TWRNINT. 1 The Tx error counter transitioned from less than 96 to greater than or equal to 96.
                     + BIT(16) // RWRNINT. 1 The Rx error counter transitioned from less than 96 to greater than or equal to 96.
                     + BIT(2)  // BOFFINT. ‘Bus Off’ Interrupt
                     + BIT(1)  // ERRINT . Error Interrupt
                     + BIT(0)  // WAKINT . Wake-Up Interrupt
                    );
  if (reg)
  {
    CAN->ESR1 = reg; // Сбрасываем флаги
  }
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
int Solve_can_timings(uint32_t bus_clock, uint32_t bitrate, T_can_ctrl *ctrl)
{
  uint32_t TQ_x_Prescaler = bus_clock / bitrate;
  uint32_t TQ;
  uint32_t lowest_diff;
  uint32_t diff;
  uint32_t best_TQ;
  uint32_t actual_freq;


  ctrl->pseg1 = 0;
  ctrl->pseg2 = 0;
  ctrl->propseg = 0;
  ctrl->rjw = 0;
  ctrl->presdiv = 0;


  // If BSP_CANPE_CLOCK is defined, then we will calculate the CAN bit timing parameters
  // using the method outlined in AN1798, section 4.1. A maximum time for PROP_SEG will be used,
  // the remaining TQ will be split equally between PSEG1 and PSEG2, provided PSEG2 >=2. RJW is
  // set to the minimum of 4 or PSEG1.


  if (TQ_x_Prescaler < (CAN_MIN_TQ - 1))
  {
    // We will be off by more than 12.5%
    return MQX_ERROR;
  }

  // First, find the best TQ and pre-scaler to use for the desired frequency. If any exact matches
  // is found, we use the match that gives us the lowest pre-scaler and highest TQ, otherwise we pick
  // the TQ and prescaler that generates the closest frequency to the desired frequency.

  lowest_diff = bitrate;
  best_TQ = 0;
  for (TQ = CAN_MAX_TQ; TQ >= CAN_MIN_TQ; TQ--)
  {
    ctrl->presdiv = TQ_x_Prescaler / TQ;
    if (ctrl->presdiv <= 256)
    {
      if (TQ_x_Prescaler == TQ * ctrl->presdiv)
      {
        best_TQ = TQ;
        break;
      }
      actual_freq = (bus_clock / ctrl->presdiv) / TQ;

      if (actual_freq > bitrate)
      {
        diff = actual_freq - bitrate;
      }
      else
      {
        diff = bitrate - actual_freq;
      }

      if (diff < lowest_diff)
      {
        lowest_diff = diff;
        best_TQ = TQ;
      }
    }
  }
  if ((best_TQ >= CAN_MIN_TQ) && (ctrl->presdiv <= 256))
  {
    ctrl->pseg2 = (best_TQ - CAN_MAX_PROPSEG) / 2;
    if (ctrl->pseg2 < CAN_MIN_PSEG2) ctrl->pseg2 = CAN_MIN_PSEG2;

    if (best_TQ == CAN_MIN_TQ)
    {
      ctrl->pseg1 = 1;
    }
    else
    {
      ctrl->pseg1 = ctrl->pseg2;
    }

    ctrl->propseg = best_TQ - ctrl->pseg1 - ctrl->pseg2 - 1;

    if (ctrl->pseg1 < CAN_MAX_RJW)
    {
      ctrl->rjw = ctrl->pseg1;
    }
    else
    {
      ctrl->rjw = CAN_MAX_RJW;
    }

    ctrl->propseg -= 1;
    ctrl->rjw -= 1;
    ctrl->pseg1 -= 1;
    ctrl->pseg2 -= 1;
    ctrl->presdiv -= 1;
  }
  else
  {
    return MQX_ERROR;
  }

  return MQX_OK;

}



/*-------------------------------------------------------------------------------------------------------------
  Создание объектов синхронизации задачи управления лифтом
-------------------------------------------------------------------------------------------------------------*/
static _mqx_uint Create_CANTX_control_sync_obj()
{
  _mqx_uint res;

  res = _lwevent_create(&can_tx_event, LWEVENT_AUTO_CLEAR);
  if (res != MQX_OK)
  {
    return res;
  }


  // Создаем очередь сообщений для отправки пакетов передатчику
  can_tx_queue_id = _msgq_open(CAN_TX_QUEUE, 0);
  if (can_tx_queue_id == MSGQ_NULL_QUEUE_ID)
  {
    res = _task_get_error();
    return res;
  }

  // Выделяем пул памяти для сообщений
  can_msg_pool_id = _msgpool_create(sizeof(T_can_tx_message), CAT_TX_QUEUE_SZ, 0, 0);
  if (can_msg_pool_id == 0)
  {
    res = _task_get_error();
  }

  return MQX_OK;
}



/*-------------------------------------------------------------------------------------------------------------
  Обслуживание прерываний CAN шины

  ptr - указатель на область регистров контроллера CAN:  CAN0_BASE_PTR или CAN1_BASE_PTR
-------------------------------------------------------------------------------------------------------------*/
int CAN_init(CAN_MemMapPtr ptr, uint32_t bitrate)
{
  _mqx_uint              res;

  SIM_MemMapPtr          SIM  = SIM_BASE_PTR;
  T_can_ctrl             ctrl;
  uint32_t           i;

  CAN_PTR = ptr;

  res = _lwevent_create(&can_rx_event, 0);
  if (res != MQX_OK)
  {
    return res;
  }
  #ifdef ENABLE_CAN_LOG
  res = _lwsem_create(&can_log_sem, 1);
  if (res != MQX_OK)
  {
    return res;
  }
  #endif

  if (ptr == CAN0_BASE_PTR)
  {
    SIM->SCGC6 |= BIT(4);  // Разрешаем тактирование CAN0
    CAN_PTR->CTRL1 |= BIT(13); // Тактирование от источника тактирования периферии

    // Инициализация всех прерываний
    _int_install_isr(INT_CAN0_ORed_Message_buffer, CAN_isr, (void *)ptr);
    _bsp_int_init(INT_CAN0_ORed_Message_buffer, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN0_Bus_Off, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN0_Bus_Off, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN0_Error, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN0_Error, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN0_Tx_Warning, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN0_Tx_Warning, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN0_Rx_Warning, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN0_Rx_Warning, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN0_Wake_Up, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN0_Wake_Up, CAN_ISR_PRIO, 0, TRUE);
  }
  else if (ptr == CAN1_BASE_PTR)
  {
    SIM->SCGC3 |= BIT(4);  // Разрешаем тактирование CAN1
    CAN_PTR->CTRL1 |= BIT(13); // Тактирование от источника тактирования периферии

    // Инициализация всех прерываний
    _int_install_isr(INT_CAN1_ORed_Message_buffer, CAN_isr, (void *)ptr);
    _bsp_int_init(INT_CAN1_ORed_Message_buffer, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN1_Bus_Off, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN1_Bus_Off, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN1_Error, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN1_Error, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN1_Tx_Warning, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN1_Tx_Warning, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN1_Rx_Warning, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN1_Rx_Warning, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN1_Wake_Up, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN1_Wake_Up, CAN_ISR_PRIO, 0, TRUE);
  }
  else return MQX_ERROR;

  // Выполняем программный сброс и переходим во Freeze mode
  if (CAN_PTR->MCR & BIT(20)) // Проверим выключен ли модуль, и сбросим этот режим если надо
  {
    CAN_PTR->MCR &= (~BIT(31));       // Включаем модуль
    while (CAN_PTR->MCR & BIT(20))    // Ждем пока флаг не сбросится
    {
    }
  }

  CAN_PTR->MCR = BIT(25);             // Выполняем SOFTRESET
  while (CAN_PTR->MCR & BIT(25))      // Ожидаем пока SOFTRESET закончится
  {
  }

  CAN_PTR->MCR |= 0
    + BIT(30) // FRZ    . Freeze Enable. 1 Enabled to enter Freeze Mode
    + BIT(28) // HALT   . Halt FlexCAN. 1 Enters Freeze Mode if the FRZ bit is asserted.
  ;
  while ((CAN_PTR->MCR & BIT(24)) == 0)   // Ожидаем установки FRZACK
  {
  }

  //Solve_can_timings(BSP_BUS_CLOCK, bitrate, &ctrl);

  if (bitrate == 333333)
  {
    // На плате DoorController тактовая частота периферии CAN = 36 000 000 Гц.
    // Частота квантов = 1800000 Длительность кванта = 0.16666... мкс
    // Частота битов = 333333 Гц
    // В бите 18 квантов, где PROP_SEG + PHASE_SEG1 = 12, PHASE_SEG2 = 5

    // Настраиваем вручную скорость CAN чтобы приспособиться к настройка DoorController. Частота тактирования CAN периферии здесь равна 60 МГц
    // Повторям настройки таймингов из платы FrontEnd
    ctrl.presdiv = 10 - 1; // Делитель частоты = 10. Получаем квант Tq = 10/60000000 = 0.16666... мкс
    ctrl.rjw = 4 - 1;  // Стандартная величина = 3 (на длину бита не влияет)
    ctrl.propseg = 4 - 1;  // Длина PROP_SEG.   3-х битное поле (фаза перед взятием отсчета)
    ctrl.pseg1 = 8 - 1;  // Длина PHASE_SEG1. 3-х битное поле (фаза перед взятием отсчета).
    ctrl.pseg2 = 5 - 1;  // Длина PHASE_SEG2. 3-х битное поле (фаза после взятия отсчета).
  }
  else if (bitrate == 100000)
  {
    // На плате DoorController тактовая частота периферии CAN = 36 000 000 Гц.
    // Частота квантов = 1800000 Длительность кванта = 0.5 мкс
    // Частота битов = 100000 Гц
    // В бите 18 квантов, где PROP_SEG + PHASE_SEG1 = 12, PHASE_SEG2 = 5

    // Настраиваем вручную скорость CAN чтобы приспособиться к настройка DoorController. Частота тактирования CAN периферии здесь равна 60 МГц
    // Повторям настройки таймингов из платы FrontEnd
    ctrl.presdiv = 33 - 1; // Делитель частоты = 30. Получаем квант Tq = 33/60000000 = 0.55 мкс
    ctrl.rjw = 4 - 1;  // Стандартная величина = 3 (на длину бита не влияет)
    ctrl.propseg = 4 - 1;  // Длина PROP_SEG.   3-х битное поле (фаза перед взятием отсчета)
    ctrl.pseg1 = 8 - 1;  // Длина PHASE_SEG1. 3-х битное поле (фаза перед взятием отсчета).
    ctrl.pseg2 = 5 - 1;  // Длина PHASE_SEG2. 3-х битное поле (фаза после взятия отсчета).
                         // Длина бита в результате получается = SYNC_SEG(всегда равен 1) + PROP_SEG + PHASE_SEG1 + PHASE_SEG2 = 18Tq = 18*0.55 = 9.9 мкс
  }

  CAN_PTR->CTRL1 = 0
    + LSHIFT(ctrl.presdiv, 24) // PRESDIV. Prescaler Division Factor
    + LSHIFT(ctrl.rjw, 22) // RJW    . Resync Jump Width
    + LSHIFT(ctrl.pseg1, 19) // PSEG1  . Phase Segment 1
    + LSHIFT(ctrl.pseg2, 16) // PSEG2  . Phase Segment 2
    + LSHIFT(1, 15) // BOFFMSK. Bus Off Mask. 1 Bus Off interrupt enabled
    + LSHIFT(1, 14) // ERRMSK . Error Mask.   1 Error interrupt enabled
    + LSHIFT(1, 13) // CLKSRC . CAN Engine Clock Source. 1 The CAN engine clock source is the peripheral clock
    + LSHIFT(0, 12) // LPB    . Loop Back Mode
    + LSHIFT(1, 11) // TWRNMSK. Tx Warning Interrupt Mask. 1 Tx Warning Interrupt enabled
    + LSHIFT(1, 10) // RWRNMSK. Rx Warning Interrupt Mask. 1 Rx Warning Interrupt enabled
    + LSHIFT(1, 7) // SMP    . CAN Bit Sampling. 0 Just one sample is used to determine the bit value
    + LSHIFT(0, 6) // BOFFREC. Bus Off Recovery. 1 Automatic recovering from Bus Off state disabled
    + LSHIFT(0, 5) // TSYN   . Timer Sync. 1 Timer Sync feature enabled
    + LSHIFT(0, 4) // LBUF   . Lowest Buffer Transmitted First. 1 Lowest number buffer is transmitted first.
    + LSHIFT(0, 3) // LOM    . Listen-Only Mode. 1 FlexCAN module operates in Listen-Only Mode.
    + LSHIFT(ctrl.propseg, 0) // PROPSEG. Propagation Segment.
  ;

  CAN_PTR->CTRL2 = 0
    + LSHIFT(1, 28) // WRMFRZ. Write-Access to Memory in Freeze mode. 1 Enable unrestricted write access to FlexCAN memory.
    + LSHIFT(0, 24) // RFFN  . Number of Rx FIFO Filters
    + LSHIFT(0x22, 19) // TASD  . Tx Arbitration Start Delay.
    + LSHIFT(0, 18) // MRP   . Mailboxes Reception Priority. 0 Matching starts from Rx FIFO and continues on Mailboxes.
    + LSHIFT(0, 17) // RRS   . Remote Request Storing. 0 Remote Response Frame is generated.
    + LSHIFT(0, 16) // EACEN . Entire Frame Arbitration Field Comparison Enable for Rx Mailboxes.0 Rx Mailbox filter’s IDE bit is always compared and RTR is never compared despite mask bits.
  ;

  for (i = 0; i < 16; i++)
  {
    CAN_PTR->RXIMR[i] = 0;    // Сбросим все маски
    CAN_PTR->MB[i].CS = 0;    // Все буфферы сообщений не активны
    CAN_PTR->MB[i].ID = 0;    //
    CAN_PTR->MB[i].WORD0 = 0; //
    CAN_PTR->MB[i].WORD1 = 0; //
  }
  //CAN->RXIMR[CAN_RX_MB1] = 0xFF000000;

  // Выходим из режима Freeze
  CAN_PTR->MCR = 0
    + LSHIFT(0, 31) // MDIS   . Module Disable. 1 Disable the FlexCAN module.
    + LSHIFT(0, 30) // FRZ    . Freeze Enable. 1 Enabled to enter Freeze Mode
    + LSHIFT(0, 29) // RFEN   . Rx FIFO Enable. 1 Rx FIFO enabled
    + LSHIFT(0, 28) // HALT   . Halt FlexCAN. 1 Enters Freeze Mode if the FRZ bit is asserted.
    + LSHIFT(0, 26) // WAKMSK . Wake Up Interrupt Mask. 1 Wake Up Interrupt is enabled.
    + LSHIFT(0, 25) // SOFTRST. Soft Reset. 1 Resets the registers affected by soft reset.
    + LSHIFT(0, 23) // SUPV   . Supervisor Mode. 1 FlexCAN is in Supervisor Mode.
    + LSHIFT(0, 22) // SLFWAK . Self Wake Up. 1 FlexCAN Self Wake Up feature is enabled.
    + LSHIFT(1, 21) // WRNEN  . Warning Interrupt Enable. 1 TWRNINT and RWRNINT bits are set when the respective error counter transitions from less than 96 to greater than or equal to 96.
    + LSHIFT(1, 17) // SRXDIS . Self Reception Disable. 1 Self reception disabled
    + LSHIFT(1, 16) // IRMQ   . Individual Rx Masking and Queue Enable. 1 Individual Rx masking and queue feature are enabled.
    + LSHIFT(0, 13) // LPRIOEN. Local Priority Enable. 1 Local Priority enabled
    + LSHIFT(0, 12) // AEN    . Abort Enable. 0 Abort disabled.  !!!Не выставлять 1, поскольку процедуры инициализации MB расчитаны на режим 0
    + LSHIFT(0, 8) // IDAM   . ID Acceptance Mode. 00 Format A: One full ID (standard and extended) per ID Filter Table element.
    + LSHIFT(16, 0) // MAXMB  . Number of the Last Message Buffer.
  ;

  return MQX_OK;
}



/*-------------------------------------------------------------------------------------------------------------
  Установка буффера на прием и инициация приема
  n    - номер буфера
  id   - идентификатор CAN пакета
  mask - фильтр-маска принимаемого идентификатора
  ext  - если расширенный формат то 1, иначе 0
  rtr  - если 1 то будет отправлен пакет вида Remote Request Frame
-------------------------------------------------------------------------------------------------------------*/
void CAN_set_rx_mbox(volatile CAN_MemMapPtr CAN, uint8_t n, uint32_t id, uint32_t mask, uint8_t ext, uint8_t rtr)
{
  // Переходим в Freeze режим поскольку маску фильтра можно изменить только в этом режиме
  CAN->MCR |= 0
    + BIT(30) // FRZ    . Freeze Enable. 1 Enabled to enter Freeze Mode
    + BIT(28) // HALT   . Halt FlexCAN. 1 Enters Freeze Mode if the FRZ bit is asserted.
  ;
  while ((CAN->MCR & BIT(24)) == 0)   // Ожидаем установки FRZACK
  {
  }

  // Сбросим флаг прерывания
  CAN->IFLAG1 |= (1 << n);

  // Инициализируем mailbox на передачу
  CAN->MB[n].CS = 0
    + LSHIFT(0x0, 24) // CODE. 0b0000:INACTIVE- MB is not active.
    + LSHIFT(1, 22) // SRR.  Substitute Remote Request. (должен быть 1 в расширенном формате)
    + LSHIFT(ext, 21) // IDE.  ID Extended Bit.           1 = расширенный формат
    + LSHIFT(rtr, 20) // RTR.  Remote Transmission Request. 1 - это mailbox с Remote Request Frame
    + LSHIFT(0, 16) // DLC.  Длина данных
  ;
  CAN->MB[n].ID = id;
  CAN->MB[n].WORD0 = 0;
  CAN->MB[n].WORD1 = 0;
  CAN->RXIMR[n] = mask; // Установим маску


  CAN->IMASK1 |= (1 << n); // Разрешаем прерывания после приема

  // Обозначаем готовность к приему
  CAN->MB[n].CS |= BIT(26); // 0b0100: EMPTY - MB is active and empty.

  CAN->MCR &= ~(BIT(30) + BIT(28)); // Выходим из режиме Freeze
  while (CAN->MCR & BIT(24))        // Ожидаем сброса FRZACK
  {
  }

}

/*-------------------------------------------------------------------------------------------------------------
  Установка буффера на отправку и инициация отправки
  n    - номер буфера
  id   - идентификатор CAN пакета
  data - буффер с данными длинной 8 байт
  len  - длина данных
  ext  - если расширенный формат то 1, иначе 0
  rtr  - если 1 то будет отправлен пакет вида Remote transmission request
-------------------------------------------------------------------------------------------------------------*/
static _mqx_uint CAN_set_tx_mbox(volatile CAN_MemMapPtr CAN, uint8_t n, uint32_t id, uint8_t *data, uint8_t len, uint8_t ext, uint8_t rtr)
{
  // Сбросим флаг прерывания
  CAN->IFLAG1 |= (1 << n);
  // Инициализируем mailbox на передачу
  CAN->MB[n].CS = 0
    + LSHIFT(0x8, 24) // CODE. 0b1000:MB is not active
    + LSHIFT(1, 22) // SRR.  Substitute Remote Request. (должен быть 1 в расширенном формате)
    + LSHIFT(ext, 21) // IDE.  ID Extended Bit.           1 = расширенный формат
    + LSHIFT(rtr, 20) // RTR.  Remote Transmission Request. 1 - это mailbox с Remote Request Frame
    + LSHIFT(len, 16) // DLC.  Длина данных
  ;
  CAN->MB[n].ID = id;
  CAN->MB[n].WORD0 = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
  CAN->MB[n].WORD1 = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];

  // Разрешаем прерывания после передачи
  CAN->IMASK1 |= (1 << n);
  // Запускаем передачу
  CAN->MB[n].CS |= BIT(26); // 0b1100: DATA - MB is a Tx Data Frame (MB RTR must be 0)

  return MQX_OK;
}


/*-------------------------------------------------------------------------------------------------------------
  Выборка данных из mailbox-а с принятым пакетом
  n    - номер буфера
-------------------------------------------------------------------------------------------------------------*/
void CAN_read_rx_mbox(volatile CAN_MemMapPtr CAN, uint8_t n, T_can_rx *rx)
{

  uint32_t          cs;
  uint32_t          w;
  volatile uint32_t tmp;

  cs = CAN->MB[n].CS;
  rx->len = (cs >> 16) & 0x0F;
  rx->ext = (cs >> 21) & 1;
  rx->rtr = (cs >> 20) & 1;
  rx->code = (cs >> 24) & 0x0F;
  rx->canid = CAN->MB[n].ID;
  w = CAN->MB[n].WORD0;
  rx->data[0] = (w >> 24) & 0xFF;
  rx->data[1] = (w >> 16) & 0xFF;
  rx->data[2] = (w >> 8) & 0xFF;
  rx->data[3] = (w >> 0) & 0xFF;
  w = CAN->MB[n].WORD1;
  rx->data[4] = (w >> 24) & 0xFF;
  rx->data[5] = (w >> 16) & 0xFF;
  rx->data[6] = (w >> 8) & 0xFF;
  rx->data[7] = (w >> 0) & 0xFF;
  tmp = CAN->TIMER; // Разблокировка Mialbox

}


#ifdef ENABLE_CAN_LOG
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
_mqx_uint CAN_wait_log_rec(_mqx_uint ticks)
{
  if (_lwevent_wait_ticks(&can_rx_event, 0x80000000, FALSE, ticks) == MQX_OK)
  {
    _lwevent_clear(&can_rx_event, 0x80000000);
    return MQX_OK;
  }
  return MQX_ERROR;
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
_mqx_uint CAN_pop_log_rec(T_can_rx *rx)
{
  _lwsem_wait(&can_log_sem);
  if (canrx_log_head != canrx_log_tail)
  {
    *rx = canrx_log[canrx_log_tail];
    canrx_log_tail++;
    if (canrx_log_tail >= CAN_RX_LOG_SZ)
    {
      canrx_log_tail = 0;
    }
    _lwsem_post(&can_log_sem);
    return MQX_OK;
  }
  _lwsem_post(&can_log_sem);
  return MQX_ERROR;
}
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
_mqx_uint CAN_push_log_rec(T_can_rx *rx)
{
  _lwsem_wait(&can_log_sem);
  canrx_log[canrx_log_head] = *rx;
  canrx_log_head++;
  if (canrx_log_head >= CAN_RX_LOG_SZ)
  {
    canrx_log_head = 0;
  }
  if (canrx_log_head == canrx_log_tail)
  {
    canrx_log_tail++;
    if (canrx_log_tail >= CAN_RX_LOG_SZ)
    {
      canrx_log_tail = 0;
    }
  }
  _lwsem_post(&can_log_sem);

  _lwevent_set(&can_rx_event, 0x80000000); // Уведомить о записи в лог
  return 0;
}
#endif

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
_mqx_uint CAN_post_packet_to_send(uint32_t canid, uint8_t *data, uint8_t len)
{
  T_can_tx_message *cmpt;

  // Выделим память для сообщения
  cmpt = (T_can_tx_message *)_msg_alloc(can_msg_pool_id);
  if (cmpt != NULL)
  {
    cmpt->header.TARGET_QID = _msgq_get_id(0, CAN_TX_QUEUE);
    cmpt->header.SIZE = sizeof(T_can_tx_message);
    cmpt->canid = canid;
    cmpt->rtr = CAN_NO_RTR;
    if (len > 0)
    {
      memcpy(cmpt->data, data, len);
    }
    cmpt->len = len;
    if (_msgq_send(cmpt) == FALSE)
    {
      return _task_get_error();
    }
    return MQX_OK;
  }
  else return MQX_ERROR;
}
/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
_mqx_uint CAN_post_rtr_packet_to_send(uint32_t canid, uint8_t *data, uint8_t len)
{
  T_can_tx_message *cmpt;

  // Выделим память для сообщения
  cmpt = (T_can_tx_message *)_msg_alloc(can_msg_pool_id);
  if (cmpt != NULL)
  {
    cmpt->header.TARGET_QID = _msgq_get_id(0, CAN_TX_QUEUE);
    cmpt->header.SIZE = sizeof(T_can_tx_message);
    cmpt->canid = canid;
    cmpt->rtr = CAN_RTR;
    if (len > 0)
    {
      memcpy(cmpt->data, data, len);
    }
    cmpt->len = len;
    if (_msgq_send(cmpt) == FALSE)
    {
      return _task_get_error();
    }
    return MQX_OK;
  }
  else return MQX_ERROR;
}


/*-------------------------------------------------------------------------------------------------------------
  Посылка пакета по линии CAN
  Extended формат, без флага RTR
-------------------------------------------------------------------------------------------------------------*/
_mqx_uint CAN_send_packet(uint32_t canid, uint8_t *data, uint8_t len, uint8_t rtr)
{
  return CAN_set_tx_mbox(CAN_PTR, CAN_TX_MB1, canid, data, len, CAN_EXTENDED_FORMAT, rtr);
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Task_CAN_Tx(uint32_t parameter)
{
  if (Create_CANTX_control_sync_obj() != MQX_OK)
  {
    return;
  }


  if (parameter != 0)
  {
    ((T_can_processing)parameter)();
  }
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Task_CAN_Rx(uint32_t parameter)
{
  uint32_t   n;


  if (parameter != 0)
  {
    ((T_can_processing)parameter)();
  }
}


