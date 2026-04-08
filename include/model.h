#ifndef MODEL_H
#define MODEL_H

#include <stddef.h>

#define TEXT_SMALL 64
#define TEXT_MEDIUM 128
#define TEXT_LARGE 256

/*
 * 天気表示ではなく「服装意思決定を代替する戦略出力」を行うためのデータモデル。
 */

typedef struct {
    char location[TEXT_SMALL];
    char date[TEXT_SMALL];
    char weather[TEXT_SMALL];
    float temp_c;
    float wind_mps;
} WeatherInput;

typedef struct {
    /* STEP1: 環境 */
    int use_bridge;
    int use_windy_place;
    int use_atrium;
    int use_high_place;
    int use_shade;
    int use_sunny;
    int use_indoor;

    /* STEP2: 移動 */
    int move_walk_lots;
    int move_bicycle;
    int move_still;

    /* STEP3: 時間変化 */
    int time_cold_morning;
    int time_hot_noon;
    int time_cold_evening;
} BehaviorInput;

typedef struct {
    float bridge_feel;
    float outdoor_feel;
    float indoor_feel;
    float min_temp;
    float max_temp;
    float diff;
} Distribution;

typedef struct {
    char strategy_main[TEXT_MEDIUM];
    char strategy_sub[TEXT_MEDIUM];
    char strategy_combined[TEXT_LARGE];
    char comment[TEXT_MEDIUM];
    char guidance[TEXT_LARGE];
    char ng_examples[TEXT_MEDIUM];
    char outfit_image[TEXT_MEDIUM];
} StrategyResult;

#endif
