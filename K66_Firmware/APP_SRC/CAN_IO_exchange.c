// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.08.24
// 12:03:16
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

/*------------------------------------------------------------------------------
  Процедура определения состояния сигнала с подавлением дребезга

  Возвращает 1 если значение сигнала изменилось, иначе 0
 ------------------------------------------------------------------------------*/
uint32_t Do_input_processing(T_input_cbl *scbl)
{
  uint8_t upper_sig, lower_sig;

  if (scbl->itype == GEN_SW)
  {
    // Обрабатываем сигнал с бистабильного датчика
    if (*scbl->p_smpl1 > scbl->log_bound) scbl->pbncf.curr = 1;
    else scbl->pbncf.curr = 0;
  }
  else if (scbl->itype == ESC_SW)
  {
    // Обрабатываем сигнал с датчика обладающего неопределенным состоянием (контакты в цепи безопасности)
    if (*scbl->p_smpl1 > scbl->log_bound) upper_sig = 1;
    else upper_sig = 0;

    if (*scbl->p_smpl2 > scbl->log_bound) lower_sig = 1;
    else lower_sig = 0;

    if ((upper_sig) && (lower_sig))
    {
      scbl->pbncf.curr = 0;
    }
    else if ((upper_sig) && (!lower_sig))
    {
      scbl->pbncf.curr = 1;
    }
    else
    {
      scbl->pbncf.curr = -1;
    }

  }
  else return 0;


  if (scbl->pbncf.init == 0)
  {
    scbl->pbncf.init = 1;
    scbl->pbncf.prev = scbl->pbncf.curr;
    *scbl->val  = scbl->pbncf.curr;
    *scbl->val_prev = *scbl->val;
    return 0;
  }

  if (scbl->pbncf.prev != scbl->pbncf.curr)
  {
    scbl->pbncf.cnt = 0;
    scbl->pbncf.prev = scbl->pbncf.curr;
  }

  if ((scbl->pbncf.curr == 0) && (scbl->pbncf.cnt == scbl->l0_time))
  {
    *scbl->val_prev = *scbl->val;
    *scbl->val = scbl->pbncf.curr;
    *scbl->flag = 1;
    scbl->pbncf.cnt++;
    return 1;
  }
  else if ((scbl->pbncf.curr == 1) && (scbl->pbncf.cnt == scbl->l1_time))
  {
    *scbl->val_prev = *scbl->val;
    *scbl->val = scbl->pbncf.curr;
    *scbl->flag = 1;
    scbl->pbncf.cnt++;
    return 1;
  }
  else if ((scbl->pbncf.curr == -1) && (scbl->pbncf.cnt == scbl->lu_time))
  {
    *scbl->val_prev = *scbl->val;
    *scbl->val = scbl->pbncf.curr;
    *scbl->flag = 1;
    scbl->pbncf.cnt++;
    return 1;
  }
  else
  {
    if (scbl->pbncf.cnt < 0XFFFFFFFFul)
    {
      scbl->pbncf.cnt++;
    }
  }

  return 0;

}
/*------------------------------------------------------------------------------
  Запаковываем значения состояний входов в пакет CAN

 \param parr       - массив с описанием упаковки
 \param sz         - размер массива
 \param canbuf     - буфер пакета CAN
 \param packlen    - количество байт в CAN пакете
 ------------------------------------------------------------------------------*/
uint32_t  CAN_pack_inputs(T_can_inp_pack *parr, uint32_t sz, uint8_t *canbuf, uint8_t *packlen)
{
  uint32_t i;
  uint8_t  len = 0;

  memset(canbuf, 0, 8);

  for (i = 0; i < sz; i++)
  {
    if (parr[i].itype == GEN_SW)
    {
      canbuf[parr[i].nbyte] = canbuf[parr[i].nbyte] | ((*parr[i].val & 1) << parr[i].nbit);
    }
    else
    {
      canbuf[parr[i].nbyte] = canbuf[parr[i].nbyte] | ((*parr[i].val & 3) << parr[i].nbit);
    }
    if ((parr[i].nbyte + 1) > len)
    {
      len = parr[i].nbyte + 1;
    }
  }

  *packlen = len;
  return 0;
}



/*------------------------------------------------------------------------------
 Распаковываем значения состояний входов из пакета CAN


 \param rx       - принятый пакет
 \param parr     - массив с описанием упаковки
 \param sz       - размер массива

 \return _mqx_uint возвращает 0 есди не было изменений, 1 - если были изменения
 ------------------------------------------------------------------------------*/
int32_t CAN_unpack_received_inputs(T_can_rx *rx, T_can_inp_pack *parr, uint32_t sz)
{
  uint32_t i;
  int8_t   v;
  int32_t  f = 0;

  for (i = 0; i < sz; i++)
  {
    if (parr[i].itype == GEN_SW)
    {
      v = (rx->data[parr[i].nbyte] >> parr[i].nbit) & 1;
    }
    else
    {
       v = (rx->data[parr[i].nbyte] >> parr[i].nbit) & 3;
       if (v == 3) v = -1;
    }
    if (*parr[i].val != v)
    {
      *parr[i].val_prev = *parr[i].val;
      *parr[i].val = v;
      *parr[i].flag = 1;
      f = 1;
    }
  }
  return f;
}

/*------------------------------------------------------------------------------
  Запаковываем значения состояний выходов в пакет CAN


 \param parr       - массив с описанием упаковки
 \param sz         - размер массива
 \param canbuf     - буфер пакета CAN
 \param packlen    - количество байт в CAN пакете
 ------------------------------------------------------------------------------*/
uint32_t  CAN_pack_outputs(T_can_out_pack *parr, uint32_t sz, uint8_t *canbuf, uint8_t *packlen)
{
  uint32_t i;
  uint8_t  len = 0;

  memset(canbuf, 0, 8);

  for (i = 0; i < sz; i++)
  {
    canbuf[parr[i].nbyte] = canbuf[parr[i].nbyte] | ((*parr[i].val & parr[i].mask) << parr[i].nbit);
    if ((parr[i].nbyte + 1) > len)
    {
      len = parr[i].nbyte + 1;
    }
  }
  *packlen = len;
  return 0;
}


/*------------------------------------------------------------------------------
 Распаковываем значения состояний выходов из пакета CAN


 \param rx       - принятый пакет
 \param parr     - массив с описанием упаковки
 \param sz       - размер массива

 \return _mqx_uint
 ------------------------------------------------------------------------------*/
uint32_t CAN_unpack_received_outputs(T_can_rx *rx, T_can_out_pack *parr, uint32_t sz)
{
  uint32_t i;
  int8_t   v;
  for (i = 0; i < sz; i++)
  {
    v = (rx->data[parr[i].nbyte] >> parr[i].nbit) & parr[i].mask;
    if (*parr[i].val != v)
    {
      *parr[i].val = v;
    }
  }
  return 0;
}
