#ifndef BUZZER_H_
#define BUZZER_H_

#include <stdio.h>
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "pins.h"

cy_rslt_t buzzer_init(float duty_cycle, uint32_t frequency);
void buzzer_on(void);
void buzzer_off(void);

#endif