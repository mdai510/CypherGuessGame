 #ifndef __TASK_BUZZER_H__
 #define __TASK_BUZZER_H__

 #include "main.h"

 #ifdef FREERTOS

 #include "drivers.h"
 #include "rtos_events.h"

 void task_buzzer(void *arg);
 #endif

 #endif // __TASK_BUZZER_H__    
