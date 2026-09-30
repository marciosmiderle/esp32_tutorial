#pragma once

#undef millis

void millis_set(unsigned long ms);
void millis_forward(unsigned long offset_ms);
void millis_backward(unsigned long offset_ms);
unsigned long custom_millis();

#define millis() custom_millis()
