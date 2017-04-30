#ifndef __CAN_CONTROL
  #define __CAN_CONTROL



  #define CAN_DEVICE  0

  #define CAN_SPEED   100000ul // Старая система управления
//  #define CAN_SPEED   333333ul // Новая система управления

  #define CAN_MIN_TQ  8
  #define CAN_MAX_TQ  25

  #define CAN_MAX_PROPSEG 8
  #define CAN_MAX_RJW 4
  #define CAN_MIN_PSEG2 2


// Майлбоксы на чтение
  #define CAN_RX_MB1     0
  #define CAN_RX_MB2     1
  #define CAN_RX_MB3     2
  #define CAN_RX_MB4     3
  #define CAN_RX_MB5     4
  #define CAN_RX_MB6     5
  #define CAN_RX_MB7     6
  #define CAN_RX_MB8     7
// Всего майлбоксов на чтение
  #define RX_MBOX_CNT    8

// Майлбоксы на передачу
  #define CAN_TX_MB1     8

// Количество сообщений в логе приемника
  #define CAN_RX_LOG_SZ          128

  #define CAT_TX_QUEUE_SZ        64

  #define CAN_RX_WAIT_MS         10  // Время ожидание приема пакета по CAN


#define CAN_EXTENDED_FORMAT      1
#define CAN_BASE_FORMAT          0
#define CAN_RTR                  1
#define CAN_NO_RTR               0

typedef struct
{
  uint32_t pseg1;
  uint32_t pseg2;
  uint32_t propseg;
  uint32_t rjw;
  uint32_t presdiv;
}
T_can_ctrl;


typedef struct
{
  uint32_t canid;     //Полученый идентификатор CAN пакета
  unsigned char  data[8]; //Буффер с данными длинной 8 байт
  unsigned char  len;     //Длина данных
  unsigned char  ext;     //Формат идентификатора(расширенный = 1, стандартный = 0)
  unsigned char  rtr;     //Признак  Remote Request Frame
  unsigned char  code;
}
T_can_rx;

typedef struct
{
  unsigned char  mb_num;
  uint32_t canid;
  uint32_t canid_mask;
  uint8_t  broadcast; // Флаг идентификатора широковещательных посылок 
} T_can_rx_config;


typedef struct
{
  MESSAGE_HEADER_STRUCT  header;
  uint32_t canid;
  uint8_t  data[8];
  uint8_t  len;
  uint8_t  rtr; // Флаг rtr в can пакете
}
T_can_tx_message;

typedef struct
{
  uint32_t tx_err_cnt;
}
T_can_statistic;


typedef void (*T_can_processing)(void);

int  CAN_init(CAN_MemMapPtr ptr, uint32_t bitrate);
void Task_CAN_Tx(uint32_t parameter);
void Task_CAN_Rx(uint32_t parameter);


_mqx_uint CAN_post_packet_to_send(uint32_t canid, uint8_t* data, uint8_t len);
_mqx_uint CAN_post_rtr_packet_to_send(uint32_t canid, uint8_t* data, uint8_t len);

_mqx_uint CAN_send_packet(uint32_t canid, uint8_t* data, uint8_t len, uint8_t rtr);
_mqx_uint CAN_all_enter_to_work_mode(void);


void      CAN_read_rx_mbox(volatile CAN_MemMapPtr CAN, uint8_t n, T_can_rx *rx);

void      CAN_set_rx_mbox(volatile CAN_MemMapPtr CAN, uint8_t n, uint32_t id, uint32_t mask, uint8_t ext, uint8_t rtr);


#ifdef ENABLE_CAN_LOG
_mqx_uint CAN_push_log_rec(T_can_rx *rx);
_mqx_uint CAN_wait_log_rec(_mqx_uint ticks);
_mqx_uint CAN_pop_log_rec(T_can_rx  *rx);
#endif

#endif
