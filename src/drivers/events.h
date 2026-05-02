#ifndef __EVENTS_H__  
 #define  __EVENTS_H__

 typedef struct events_t
{
    unsigned int sw1 : 1;               /* Falling edge of SW1 detected */
    unsigned int sw2 : 1;               /* Falling edge of SW2 detected */
    unsigned int sw3 : 1;               /* Falling edge of SW3 detected */
    unsigned int tmr_msec_0100 : 1;     /* Timer 100mS has occurred */
    unsigned int tmr_msec_1000 : 1;    /* Timer 1 second has occurred */
} events_t;

extern volatile events_t Events;

 #endif