// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.07.07
// 15:07:23
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


extern volatile CAN_MemMapPtr        CAN_PTR;
extern LWEVENT_STRUCT                can_rx_event;
extern LWEVENT_STRUCT                can_tx_event;
extern uint32_t                      can_events_mask;
extern T_can_statistic               can_stat;
extern _queue_id                     can_tx_queue_id;
extern const T_can_rx_config         rx_mboxes[RX_MBOX_CNT];

T_can_cbl            can_cbl;
static uint32_t      g_node_number;

static T_CAN_cmd_man CAN_Cmd_man_func;

/*-----------------------------------------------------------------------------------------------------
 
 \param val 
-----------------------------------------------------------------------------------------------------*/
void U3HB01_Set_CAN_node_number(uint32_t val)
{
  g_node_number = val;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
uint32_t U3HB01_Get_CAN_node_number(void)
{
  return g_node_number;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void _MDC01_CAN_set_rx_mboxes(void)
{

  uint32_t n;

  

  // Инициализация всех майлбоксов предназначенных для приема сообщений
  for (n = 0; n < RX_MBOX_CNT; n++)
  {
    if (rx_mboxes[n].broadcast != 0)
    {
      CAN_set_rx_mbox(CAN_PTR, rx_mboxes[n].mb_num, rx_mboxes[n].canid, rx_mboxes[n].canid_mask, 1, 0);
    }
    else
    {
      CAN_set_rx_mbox(CAN_PTR, rx_mboxes[n].mb_num, rx_mboxes[n].canid | g_node_number, rx_mboxes[n].canid_mask, 1, 0);
    }
    can_events_mask |= (1 << rx_mboxes[n].mb_num); // Заносим позиционный бит этого майлбокса в маску флагов для приемника
  }

}

/*-----------------------------------------------------------------------------------------------------
 
 \param func 
-----------------------------------------------------------------------------------------------------*/
void U3HB01_Set_CAN_cmd_man(T_CAN_cmd_man func)
{
  CAN_Cmd_man_func = func;
}


/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
void U3HB01_CAN_tx(void)
{
  T_can_tx_message *cmpt;

  while (1)
  {
    // Извлекаем сообщения из очереди
    cmpt = _msgq_receive(can_tx_queue_id, 0);
    if (cmpt != NULL)
    {
      CAN_send_packet(cmpt->canid, cmpt->data, cmpt->len, cmpt->rtr);
      if (_lwevent_wait_ticks(&can_tx_event, BIT(CAN_TX_MB1), FALSE, 100) != MQX_OK)
      {
        can_stat.tx_err_cnt++;
      }
      _msg_free(cmpt);
    }
    else _time_delay_ticks(2); // Некоторая пауза если ошибки очереди начнут идти непрерывно
  }
}
/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
void U3HB01_CAN_rx(void)
{
  T_can_rx  rx;
  _mqx_uint events;
  uint32_t  n;
  uint32_t  wticks = Conv_ms_to_ticks(CAN_RX_WAIT_MS);;

  _MDC01_CAN_set_rx_mboxes();

  while (1)
  {

    if (_lwevent_wait_ticks(&can_rx_event, can_events_mask, FALSE, wticks) == MQX_OK)
    {
      events = _lwevent_get_signalled();
      _lwevent_clear(&can_rx_event, events);

      for (n = 0; n < RX_MBOX_CNT; n++) // Проходим по всем мйлбоксам CAN контроллера чтобы узнать у кого есть сообщение
      {
        if (events & (1 << rx_mboxes[n].mb_num))
        {
          // Читаем сообщение из майлбокса CAN контроллера
          can_cbl.rx_active = 1;
          can_cbl.rx_pack_cnt++;

          CAN_read_rx_mbox(CAN_PTR, rx_mboxes[n].mb_num, &rx);
          #ifdef ENABLE_CAN_LOG
          CAN_push_log_rec(&rx);
          #endif
          if (((rx.canid & 0xFF0FFFFF) == DMC01_REQ) && (rx.len == 1))
          {
            can_cbl.rx_req_cnt++;
            // Получили запрос
            // Длина запроса не должна превышать одного байта
            if (can_cbl.disable_can == 0)
            {
              int32_t ret = -1;
              if (CAN_Cmd_man_func != 0)
              {
                ret = CAN_Cmd_man_func(&rx);
              }

              if (ret < 0)
              {
                switch (rx.data[0])
                {
                case DMC01_REQ_STATUS:
                  memcpy(&rx.data[1], &g_app_errors, sizeof(g_app_errors));
                  CAN_post_packet_to_send(DMC01_ANS | g_node_number, rx.data, sizeof(g_app_errors) + 1);
                  break;
                }
              }
            }
          }
        }
      }
    }
  }

}


