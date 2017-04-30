#ifndef WATCHDOG_H
  #define WATCHDOG_H


void           WatchDog_init(unsigned int wdt_timeout, unsigned int wdt_win);
void           WatchDog_refresh(void);
unsigned short WatchDog_get_counter(void);
unsigned int   WatchDog_get_timeout_reg(void);

#endif // WATCHDOG_H



