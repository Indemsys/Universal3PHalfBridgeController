#ifndef CRC_UTILS_H
  #define CRC_UTILS_H


uint16_t Get_CCITT_CRC(uint16_t crc,uint8_t b);
uint16_t Get_CCITT_CRC_of_block(void* b,uint32_t len, uint16_t crc );
uint16_t Get_CRC_of_block(void* b,uint32_t len, uint16_t crc );



#endif // CRC_UTILS_H



