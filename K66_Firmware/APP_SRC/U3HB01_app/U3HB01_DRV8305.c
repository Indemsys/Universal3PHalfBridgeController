// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-06
// 15:05:01
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "app.h"




T_DRV8305_registers DRV8305_regs;

const T_DRV8305_map DRV8305_map[DRV8305_REGS_CNT] =
{
  { 1,  "Warning and Watchdog Reset                       ",&DRV8305_regs.WAR_WDTR,   0x0000 },
  { 2,  "OV(Over Voltage)/VDS(Voltage Drain-Source) Faults",&DRV8305_regs.OV_FAULTS,  0x0000 },
  { 3,  "IC Faults                                        ",&DRV8305_regs.IC_FAULTS,  0x0000 },
  { 4,  "VGS (Voltage Gate-Source) Faults                 ",&DRV8305_regs.VGS_FAULTS, 0x0000 },
  { 5,  "HS Gate Drive Control                            ",&DRV8305_regs.HSG_CTRL,   0x0388 },
  { 6,  "LS Gate Drive Control                            ",&DRV8305_regs.LSG_CTRL,   0x0388 },
  { 7,  "Gate Drive Control                               ",&DRV8305_regs.G_CTRL,     0x0196 }, // 8..7 - b'11 - PWM with 6 independent inputs, 9 - diode freewheeling
  { 8,  "Reserved                                         ",&DRV8305_regs.RESV,       0x0000 },
  { 9,  "IC Operation                                     ",&DRV8305_regs.IC_OP,      0x0020 },
  { 10, "Shunt Amplifier Control                          ",&DRV8305_regs.SHUNT_CTRL, SHUNT_DEF_GAIN_CODE }, // Gain of CS amplifier
  { 11, "Voltage Regulator Control                        ",&DRV8305_regs.VR_CTRL,    0x010A },
  { 12, "VDS (Voltage Drain-Source)Sense Control          ",&DRV8305_regs.VDS_CTRL,   0x02C8 },
};


#define CALIBR_SMPL_NUM 3000  // Количество отсчетов для калибровки каналов измерения тока

static uint32_t calibr_acc_a;
static uint32_t calibr_acc_b;
static uint32_t calibr_acc_c;
static uint32_t calibr_cnt;
static uint32_t shunt_amp_enabled = 0;

static int32_t  ia;
static int32_t  ib;
static int32_t  ic;
static int32_t  idc;

const int32_t*   currents_selector[6] = 
{
//  &ib,
//  &ic,
//  &ic,
//  &ia,
//  &ia,
//  &ib 

  &ia,
  &ia,
  &ib,
  &ib,
  &ic,
  &ic 

};

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
_mqx_uint DRV8305_init(void)
{

  if (DRV8305_read_all() != MQX_OK) return MQX_ERROR;
  if (DRV8305_write_def_all() != MQX_OK) return MQX_ERROR;
  return MQX_OK;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
 
 \return T_DRV8305_map* 
-----------------------------------------------------------------------------------------------------*/
const T_DRV8305_map* DRV8305_get_map(uint32_t  *psz)
{
  if (psz != 0) *psz = DRV8305_REGS_CNT;
  return DRV8305_map;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param addr 
 \param pval 
-----------------------------------------------------------------------------------------------------*/
_mqx_uint DRV8305_read_register(uint8_t addr, uint16_t *pval)
{
  _mqx_uint res = MQX_OK;
  uint16_t  cmdw;

  cmdw = BIT(15);
  cmdw |= ((addr & 0x0F) << 11);

  res = SPISysBus_read_16bit_word(SPI_CS1, cmdw,pval);
  if (res != MQX_OK) g_app_errors |= APP_ERR_NO_DRV8305;
  return res;

}

/*-----------------------------------------------------------------------------------------------------
 
 \param addr 
 \param pval 
-----------------------------------------------------------------------------------------------------*/
_mqx_uint DRV8305_write_register(uint8_t addr, uint16_t val)
{
  _mqx_uint res = MQX_OK;
  uint16_t  cmdw;

  cmdw = ((addr & 0x0F) << 11);
  cmdw |=  val & 0x7FF;

  res = SPISysBus_write_16bit_word(SPI_CS1, cmdw);
  if (res != MQX_OK) g_app_errors |= APP_ERR_NO_DRV8305;
  return res;
}
/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
_mqx_uint DRV8305_read_all(void)
{
  uint32_t i;

  for (i = 0; i < DRV8305_REGS_CNT; i++)
  {
    if (DRV8305_read_register(DRV8305_map[i].addr, DRV8305_map[i].pval) != MQX_OK)
    {
      LOG("DRV8305 read error.", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
      return MQX_ERROR;
    }
  }
  return MQX_OK;
}

/*-----------------------------------------------------------------------------------------------------
 Записать во все регистры инициализационые значения
 \param void 
-----------------------------------------------------------------------------------------------------*/
_mqx_uint DRV8305_write_def_all(void)
{
  uint32_t i;

  for (i = 0; i < DRV8305_REGS_CNT; i++)
  {
    if (DRV8305_write_register(DRV8305_map[i].addr, DRV8305_map[i].defv) != MQX_OK)
    {
      LOG("DRV8305 write error.", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
      return MQX_ERROR;
    }
  }
  return MQX_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Чтение регистров статуса с кодами ошибок драйвера  
 
 \param void 
 
 \return _mqx_uint 
-----------------------------------------------------------------------------------------------------*/
_mqx_uint DRV8305_read_status_regs(void)
{
  uint32_t i;

  for (i = 0; i < 4; i++)
  {
    if (DRV8305_read_register(DRV8305_map[i].addr, DRV8305_map[i].pval) != MQX_OK)
    {
      LOG("DRV8305 read error.", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
      return MQX_ERROR;
    }
  }
  if (DRV8305_regs.WAR_WDTR & BIT(10)) // Проверка флага Fault indication
  {
    g_app_errors |= APP_ERR_DRV8305_FAULT;
    return MQX_ERROR;
  }
  return MQX_OK;
}

/*-----------------------------------------------------------------------------------------------------
 Включаем управление выходами у драйвера мотора  DRV8305.
 Сигнал EN_GATE у DRV8305
 После этого начинают работу выходные драйверы и шунтовые усилители тока 
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void DRV8305_Gates_enable(void)
{
  PTA_BASE_PTR->PSOR = BIT(25);
  shunt_amp_enabled = 1;
  //_time_delay(10); // Нужна задержка для того чтобы зарядились цепи управления драйверами
  // Но здесь ее делать нельзя поскольку эта функция вызывается в прерываниях
}
/*-----------------------------------------------------------------------------------------------------
 Отключаем управление выходами у драйвера мотора  DRV8305.
 Сигнал EN_GATE у DRV8305
 После этого прекращают работу выходные драйверы и шунтовые усилители тока 
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void DRV8305_Gates_disable(void)
{
  shunt_amp_enabled = 0;
  PTA_BASE_PTR->PCOR = BIT(25);
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается после окончания преобразования ADC
  Прерывание уровня ядра!!!
  Нельзя использовать сервисы RTOS!
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void DRV8305_calibrate_callback(void)
{
  if (calibr_cnt < CALIBR_SMPL_NUM)
  {
    calibr_cnt++;
    calibr_acc_a += adcs.smpl_SNS_IA;
    calibr_acc_b += adcs.smpl_SNS_IB;
    calibr_acc_c += adcs.smpl_SNS_IC;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Получить токи в формате с плавающей запятой из значений выборок 
  Положительный ток - когда ток течет через шунтовой резистор к его терминалу земли (втекает в землю схемы)  
  Отрицательный ток - когда ток течет через шунтовой резистор из его терминала земли  (вытекает из земли схемы)
 
 \param void 
 
 \return float 
-----------------------------------------------------------------------------------------------------*/
static T_median_filter_uint16 ia_filter;
static T_median_filter_uint16 ib_filter;
static T_median_filter_uint16 ib_filter;
static T_median_filter_uint32 idc_filter;

static int32_t  ia_acc;
static int32_t  ib_acc;
static int32_t  ic_acc;
static uint32_t curr_acc_cnt;
static uint8_t  clear_acc_fl;

static T_currents scrnts;


/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void DRV8305_clear_currents_acc(void)
{
  clear_acc_fl = 1;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
 
 \return T_currents* 
-----------------------------------------------------------------------------------------------------*/
T_currents* DRV8305_get_shunt_currents(void)
{
  return &scrnts;
}

/*-----------------------------------------------------------------------------------------------------
  Процедура измерения токов с частотой модуляции
  Вызывается после окончания преобразования ADC
  Прерывание уровня ядра!!!
  Нельзя использовать сервисы RTOS!

 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void  DRV8305_currents_measuring(void)
{

  if (shunt_amp_enabled == 0)
  {
    // Привыключеном драйвере мотора усилители токов шунтов не работают, и считаем что токи нулевые
    ia = 0;
    ib = 0;
    ic = 0;
    current_a = 0;
    current_b = 0;
    current_c = 0;
  }
  else
  {
    // Фильтруем отсчеты тока медианным фильтром 
    // ia = (int32_t)MedianFilter_3uint16(adcs.smpl_SNS_IA, &ia_filter);
    // ib = (int32_t)MedianFilter_3uint16(adcs.smpl_SNS_IB, &ib_filter);
    // ic = (int32_t)MedianFilter_3uint16(adcs.smpl_SNS_IC, &ib_filter);

    // Вариант без фильтрации 
    ia = (int32_t)adcs.smpl_SNS_IA;
    ib = (int32_t)adcs.smpl_SNS_IB;
    ic = (int32_t)adcs.smpl_SNS_IC;

    ia = (ia - (int32_t)sns_ia_calibr);
    ib = (ib - (int32_t)sns_ib_calibr);
    ic = (ic - (int32_t)sns_ic_calibr);

    current_a = (float)ia * SHUNT_CURRENT_TO_FLOAT;
    current_b = (float)ib * SHUNT_CURRENT_TO_FLOAT;
    current_c = (float)ic * SHUNT_CURRENT_TO_FLOAT;

    // Находим значение общего тока через драйверы
    idc = *currents_selector[bldcs_cbl.commutation_step];
    //idc = (int32_t)MedianFilter_3uint32(idc, &idc_filter);

    dc_current = (float)idc * SHUNT_CURRENT_TO_FLOAT;
  }

  // Аккумулируем абсолютные значения токов
  if (clear_acc_fl != 0)
  {
    scrnts.ia_acc = ia_acc;
    scrnts.ib_acc = ib_acc;
    scrnts.ic_acc = ic_acc;
    scrnts.curr_acc_cnt = curr_acc_cnt;

    ia_acc = abs(ia);
    ib_acc = abs(ib);
    ic_acc = abs(ic);
    curr_acc_cnt = 1;
    clear_acc_fl = 0;
  }
  else
  {
    ia_acc += abs(ia);
    ib_acc += abs(ib);
    ic_acc += abs(ic);
    curr_acc_cnt++;
  }


}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void DRV8305_calibrate(void)
{


   DRV8305_write_register(10, 0x700 | SHUNT_DEF_GAIN_CODE); // Переключаем в чип режим калибровки
  //DRV8305_write_register(10, SHUNT_DEF_GAIN_CODE); // Не переключаем в чип режим калибровки

  calibr_acc_a = 0;
  calibr_acc_b = 0;
  calibr_acc_c = 0;
  calibr_cnt = 0;
  // Регистрируем callback функцию
  Set_adc_isr_current_meas_callback(DRV8305_calibrate_callback);

  // Ждем пока не накопится необходимое количество отсчетов для усреднения
  while (calibr_cnt < CALIBR_SMPL_NUM)
  {
    _time_delay(10);
  }

  sns_ia_calibr = calibr_acc_a / CALIBR_SMPL_NUM;
  sns_ib_calibr = calibr_acc_b / CALIBR_SMPL_NUM;
  sns_ic_calibr = calibr_acc_c / CALIBR_SMPL_NUM;

  // Снимаем регистрацию
  Set_adc_isr_current_meas_callback(0);

  DRV8305_write_register(10, SHUNT_DEF_GAIN_CODE); // Возвращаем чип в нормальный режим измерения

  ia_acc       = 0;
  ib_acc       = 0;
  ic_acc       = 0;
  curr_acc_cnt = 0;
  clear_acc_fl = 0;
  Set_adc_isr_current_meas_callback(DRV8305_currents_measuring);

  _time_get(&g_calibr_time);
}


