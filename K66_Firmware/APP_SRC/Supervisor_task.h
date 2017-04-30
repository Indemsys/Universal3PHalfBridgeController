#ifndef SUPERVISOR_TASK_H
  #define SUPERVISOR_TASK_H


// Задание интервалов времени для управления watchdog в мс
#define WATCHDOG_TIMEOUT     250  // Верхняя граница времени сброса watchdog
#define WATCHDOG_WIN         50   // Нижняя граница времени сброса watchdog
#define SUPERVISOR_TIMEOUT   100  // Период времени в мс по истечении которого задача Task_supervisor сбрасывает watchdog


void Task_supervisor(unsigned int initial_data);

void Write_start_log_rec(void);

#endif // SUPERVISOR_TASK_H



