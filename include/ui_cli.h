#ifndef UI_CLI_H
#define UI_CLI_H

#include "model.h"

void run_quick_mode(const WeatherInput* w);
void run_precise_mode(const WeatherInput* w, BehaviorInput* b);
void collect_weather_input(WeatherInput* w);

#endif
