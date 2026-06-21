/*
 * button.c
 *
 *  Created on: Jun 20, 2026
 *      Author: tomas
 */

#include "button.h"
#include <stdint.h>

void Button_Debounce(volatile Button_t *btn) {
	if (btn->gpio_state != btn->prev_gpio_state) {
		btn->gpio_counter = 0;
		btn->prev_gpio_state = btn->gpio_state;
	} else {
		btn->gpio_counter++;
	}

	if (btn->gpio_counter >= 20 && btn->gpio_state == GPIO_PIN_SET) {
		btn->button_pressed = 1;
		btn->gpio_counter = 0;
	}
}

uint8_t Button_IsPressed(volatile Button_t *btn) {
	if (btn->button_pressed == 1) {
		btn->button_pressed = 0;
		return 1;

	}
	return 0;
}
