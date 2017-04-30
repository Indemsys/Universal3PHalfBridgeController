#ifndef U3HB01_DRV8305_H
#define U3HB01_DRV8305_H



#define  DRV8305_REGS_CNT 12


#define  SHUNT_GAIN_80  0x3F
#define  SHUNT_GAIN_40  0x2A
#define  SHUNT_GAIN_20  0x15
#define  SHUNT_GAIN_10  0x00

//#define  SHUNT_DEF_GAIN_CODE    SHUNT_GAIN_40
#define  SHUNT_DEF_GAIN_CODE    SHUNT_GAIN_80
//#define  SHUNT_DEF_GAIN         40.0
#define  SHUNT_DEF_GAIN         80.0
#define  SHUNT_VREF             (3.3/2.0)


#define  SHUNT_RES  (0.001)
#define  SHUNT_CURRENT_TO_FLOAT (VREF / (SHUNT_DEF_GAIN * ADC_PREC * SHUNT_RES))

typedef struct
{
  uint16_t  WAR_WDTR;  // Warning and Watchdog Reset
  uint16_t  OV_FAULTS; // OV(Over Voltage)/VDS(Voltage Drain-Source) Faults
  uint16_t  IC_FAULTS; // IC Faults
  uint16_t  VGS_FAULTS; // VGS (Voltage Gate-Source) Faults
  uint16_t  HSG_CTRL;  // HS Gate Drive Control
  uint16_t  LSG_CTRL;  // LS Gate Drive Control
  uint16_t  G_CTRL;    // Gate Drive Control
  uint16_t  RESV;      // Reserved
  uint16_t  IC_OP;     // IC Operation
  uint16_t  SHUNT_CTRL; // Shunt Amplifier Control
  uint16_t  VR_CTRL;   // Voltage Regulator Control
  uint16_t  VDS_CTRL;  // VDS (Voltage Drain-Source)Sense Control


} T_DRV8305_registers;


typedef struct
{
  uint8_t       addr;   // Адрес регистра в чипе
  const char    *vname; // Имя переменной
  uint16_t      *pval;  // Указатель на переменную содержащую  значение регистра
  uint16_t      defv;   // Инициализационная сонстанта
} T_DRV8305_map;


typedef struct
{
  int32_t  ia_acc;
  int32_t  ib_acc;
  int32_t  ic_acc;
  uint32_t curr_acc_cnt;

} T_currents;



_mqx_uint      DRV8305_init(void);
_mqx_uint      DRV8305_read_register(uint8_t addr, uint16_t *pval);
_mqx_uint      DRV8305_write_register(uint8_t addr, uint16_t val);
_mqx_uint      DRV8305_read_all(void);
_mqx_uint      DRV8305_write_def_all(void);
_mqx_uint      DRV8305_read_status_regs(void);
const T_DRV8305_map* DRV8305_get_map(uint32_t  *psz);
void           DRV8305_calibrate(void);

void           DRV8305_clear_currents_acc(void);
T_currents*    DRV8305_get_shunt_currents(void);
void           DRV8305_Gates_enable(void);
void           DRV8305_Gates_disable(void);

#endif // MDC01_DRV8305_H



