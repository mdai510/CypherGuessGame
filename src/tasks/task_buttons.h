 #ifndef __TASK_BUTTONS_H__
 #define __TASK_BUTTONS_H__

 #include "main.h"

#ifdef FREERTOS

 #include "drivers.h"
 #include "rtos_events.h"

 void task_buttons(void *arg);
 bool task_buttons_init(void);
 #endif

#endif // __TASK_BUTTONS_H__
