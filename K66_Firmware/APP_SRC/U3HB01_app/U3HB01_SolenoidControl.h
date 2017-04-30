#ifndef U3HB01_SOLENOIDCONTROL_H
#define U3HB01_SOLENOIDCONTROL_H

#define EVT_SOLENOID_ON          BIT(0)
#define EVT_SOLENOID_OFF         BIT(1)


void     U3HB01_Slnd_post_evt(_mqx_uint evts);
uint32_t U3HB01_Solenoid_control(void);


#endif // MDC01_SOLENOIDCONTROL_H



