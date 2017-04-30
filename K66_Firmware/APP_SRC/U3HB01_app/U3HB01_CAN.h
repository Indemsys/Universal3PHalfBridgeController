#ifndef U3HB01_CAN_H
  #define U3HB01_CAN_H


typedef struct
{
  uint8_t   rx_active;    // ‘лаг активности на шине CAN  
  uint32_t  rx_pack_cnt;  // —четчик прин€тых пакетов 
  uint32_t  rx_req_cnt;   // —четчик прин€тых пакетов типа REQ 
  uint8_t   disable_can;
} T_can_cbl;

typedef int32_t(*T_CAN_cmd_man)(T_can_rx *);

void     U3HB01_CAN_tx(void);
void     U3HB01_CAN_rx(void);
void     U3HB01_Set_CAN_cmd_man(T_CAN_cmd_man func);
void     U3HB01_Set_CAN_node_number(uint32_t val);
uint32_t U3HB01_Get_CAN_node_number(void);

#endif // RB150V2PC_CAN_H



