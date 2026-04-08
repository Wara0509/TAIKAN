#ifndef MODEL_H
#define MODEL_H

/*
 * STEP1では構造体の宣言のみ行い、詳細ロジックはSTEP2以降で実装する。
 */

typedef struct {
    const char* location;
    const char* date;
    const char* weather;
    float temp_c;
    float wind_mps;
} WeatherInput;

typedef struct {
    int use_bridge;
    int use_windy_place;
    int use_atrium;
    int use_high_place;
    int use_shade;
    int use_sunny;
    int use_indoor;

    int move_walk_lots;
    int move_bicycle;
    int move_still;

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
    char strategy_main[64];
    char strategy_sub[128];
    char guidance[256];
    char ng_examples[128];
    char outfit_image[128];
} StrategyResult;

#endif
