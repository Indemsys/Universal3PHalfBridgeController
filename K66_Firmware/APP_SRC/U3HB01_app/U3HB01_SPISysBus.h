#ifndef U3HB01_SPISYSBUS_H
#define U3HB01_SPISYSBUS_H


_mqx_uint SPISysBus_init(uint32_t en_dma);
_mqx_uint SPISysBus_read_16bit_word(uint32_t cs, uint16_t outv, uint16_t *pinv);
_mqx_uint SPISysBus_write_16bit_word(uint32_t cs, uint16_t val);
_mqx_uint SPISysBus_write_read_buf_dma(void *wbuff,  void *rbuff, uint32_t sz);


#endif // U3HB01_SPISYSBUS_H



