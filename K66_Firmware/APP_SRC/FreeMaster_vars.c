// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-28
// 22:49:21
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "freemaster_tsa.h"

extern T_DRV8305_registers   DRV8305_regs;
extern T_can_cbl             can_cbl;
extern float                 slnd_pwr;
extern uint8_t               slnd_en;

FMSTR_TSA_TABLE_BEGIN(tbl_vars)

FMSTR_TSA_RW_VAR(cpu_usage                               , FMSTR_TSA_UINT32) 

FMSTR_TSA_RW_VAR(slnd_pwr                                , FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(slnd_en                                 , FMSTR_TSA_UINT8 )

FMSTR_TSA_RW_VAR(wvar.dev_task                           , FMSTR_TSA_UINT8 )
FMSTR_TSA_RW_VAR(wvar.rot_dir                            , FMSTR_TSA_UINT8 )
FMSTR_TSA_RW_VAR(wvar.rot_fbreak_pwm_lev                 , FMSTR_TSA_UINT8 )
FMSTR_TSA_RW_VAR(wvar.rot_pwm_lev                        , FMSTR_TSA_UINT8 )
FMSTR_TSA_RW_VAR(wvar.rot_steps                          , FMSTR_TSA_UINT32) 

FMSTR_TSA_RW_VAR(g_app_errors                            , FMSTR_TSA_UINT32) 
FMSTR_TSA_RW_VAR(g_bldc_active                           , FMSTR_TSA_UINT32) 
                                                         
FMSTR_TSA_RW_VAR(bldcs_cbl.rot_dir                       , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(bldcs_cbl.modulation                    , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(bldcs_cbl.commutation_step              , FMSTR_TSA_UINT8 )
FMSTR_TSA_RW_VAR(bldcs_cbl.a_out_level                   , FMSTR_TSA_SINT8 ) 
FMSTR_TSA_RW_VAR(bldcs_cbl.b_out_level                   , FMSTR_TSA_SINT8 ) 
FMSTR_TSA_RW_VAR(bldcs_cbl.c_out_level                   , FMSTR_TSA_SINT8 ) 

FMSTR_TSA_RW_VAR(hccbl.ftm2_overfl                       , FMSTR_TSA_UINT32) 
FMSTR_TSA_RW_VAR(hccbl.hall_pulse_len                    , FMSTR_TSA_UINT32) 
FMSTR_TSA_RW_VAR(hccbl.prev_cnv                          , FMSTR_TSA_UINT32) 
FMSTR_TSA_RW_VAR(hccbl.prev_hst                          , FMSTR_TSA_UINT32) 
FMSTR_TSA_RW_VAR(hccbl.hall_counter                      , FMSTR_TSA_SINT32) 
FMSTR_TSA_RW_VAR(hccbl.undef_state_errors                , FMSTR_TSA_UINT32) 
FMSTR_TSA_RW_VAR(hccbl.big_delta_errors                  , FMSTR_TSA_UINT32) 
FMSTR_TSA_RW_VAR(hccbl.no_change_state_errors            , FMSTR_TSA_UINT32) 
FMSTR_TSA_RW_VAR(hccbl.overfl_bound                      , FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(hccbl.speed_filter_en                   , FMSTR_TSA_UINT8 )
                                                         
FMSTR_TSA_RW_VAR(bldcs_cbl.dir_inv                       , FMSTR_TSA_UINT8 ) 

                                                         
FMSTR_TSA_RW_VAR(hall_inputs                             , FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(mot_PWM_lev                             , FMSTR_TSA_SINT16)
                                                         
FMSTR_TSA_RW_VAR(adcs.smpl_SNS_IA                        , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adcs.smpl_U_A                           , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adcs.smpl_U_B                           , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adcs.smpl_UVDD                          , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adcs.smpl_TEMP                          , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adcs.smpl_SNS_IB                        , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adcs.smpl_SNS_IC                        , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adcs.smpl_U_C                           , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adcs.smpl_EXTTEMP                       , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adcs.smpl_Temper3                       , FMSTR_TSA_UINT16)
                                                         
FMSTR_TSA_RW_VAR(ovrc_cbl.aver_curr_a                    , FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(ovrc_cbl.aver_curr_b                    , FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(ovrc_cbl.aver_curr_c                    , FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(ovrc_cbl.max_current                    , FMSTR_TSA_FLOAT ) 
FMSTR_TSA_RW_VAR(ovrc_cbl.max_long_current               , FMSTR_TSA_FLOAT ) 
FMSTR_TSA_RW_VAR(ovrc_cbl.long_aver_curr_a               , FMSTR_TSA_FLOAT ) 
FMSTR_TSA_RW_VAR(ovrc_cbl.long_aver_curr_b               , FMSTR_TSA_FLOAT ) 
FMSTR_TSA_RW_VAR(ovrc_cbl.long_aver_curr_c               , FMSTR_TSA_FLOAT ) 
FMSTR_TSA_RW_VAR(ovrc_cbl.overload_flags                 , FMSTR_TSA_UINT8 )  

                                                         
FMSTR_TSA_RW_VAR(current_a                               , FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(current_b                               , FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(current_c                               , FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(dc_current                              , FMSTR_TSA_FLOAT)
                               
FMSTR_TSA_RW_VAR(int_temp                                , FMSTR_TSA_FLOAT) 
FMSTR_TSA_RW_VAR(drv_temp                                , FMSTR_TSA_FLOAT) 
FMSTR_TSA_RW_VAR(motor_temp                              , FMSTR_TSA_FLOAT)       
FMSTR_TSA_RW_VAR(dc_voltage                              , FMSTR_TSA_FLOAT) 
FMSTR_TSA_RW_VAR(mot_speed_rps                           , FMSTR_TSA_FLOAT) 
FMSTR_TSA_RW_VAR(mot_speed_rps_raw                       , FMSTR_TSA_FLOAT) 
                                       
FMSTR_TSA_RW_VAR(DRV8305_regs.WAR_WDTR                   , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(DRV8305_regs.OV_FAULTS                  , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(DRV8305_regs.IC_FAULTS                  , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(DRV8305_regs.VGS_FAULTS                 , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(DRV8305_regs.HSG_CTRL                   , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(DRV8305_regs.LSG_CTRL                   , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(DRV8305_regs.G_CTRL                     , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(DRV8305_regs.RESV                       , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(DRV8305_regs.IC_OP                      , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(DRV8305_regs.SHUNT_CTRL                 , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(DRV8305_regs.VR_CTRL                    , FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(DRV8305_regs.VDS_CTRL                   , FMSTR_TSA_UINT16)
                                                         
FMSTR_TSA_RW_VAR(can_cbl.rx_active                       , FMSTR_TSA_UINT8 ) 
FMSTR_TSA_RW_VAR(can_cbl.rx_pack_cnt                     , FMSTR_TSA_UINT32) 
FMSTR_TSA_RW_VAR(can_cbl.rx_req_cnt                      , FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(can_cbl.disable_can                     , FMSTR_TSA_UINT8 )

FMSTR_TSA_RW_VAR(g_fmstr_rate_src                        , FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(g_fmstr_smpls_period                    , FMSTR_TSA_FLOAT )

FMSTR_TSA_RW_VAR(ext_enc_angle                           , FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(ext_enc_errors                          , FMSTR_TSA_UINT32)

FMSTR_TSA_TABLE_END()
;

