#ifndef LOGIC_H
#define LOGIC_H

#include "model.h"

float calculate_feels_like(float base_temp, float wind_mps, const BehaviorInput* b);
void calculate_distribution(float base_temp, float wind_mps, const BehaviorInput* b, Distribution* d);
void analyze_strategy(const Distribution* d, float wind_mps, const BehaviorInput* b, StrategyResult* s);
void generate_output(const WeatherInput* w, const Distribution* d, const StrategyResult* s);

#endif
