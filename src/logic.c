#include "logic.h"

#include <float.h>
#include <stdio.h>
#include <string.h>

static float wind_correction(float wind_mps) {
    if (wind_mps >= 8.0f) {
        return 4.0f;
    }
    if (wind_mps >= 5.0f) {
        return 2.0f;
    }
    if (wind_mps >= 3.0f) {
        return 1.0f;
    }
    return 0.0f;
}

static float place_correction(const BehaviorInput* b) {
    float correction = 0.0f;

    if (b->use_bridge) {
        correction -= 1.5f;
    }
    if (b->use_high_place) {
        correction -= 1.0f;
    }
    if (b->use_atrium) {
        correction -= 1.0f;
    }
    if (b->use_shade) {
        correction -= 0.5f;
    }
    if (b->use_sunny) {
        correction += 1.0f;
    }

    return correction;
}

static float activity_correction(const BehaviorInput* b) {
    if (b->move_walk_lots || b->move_bicycle) {
        return 1.5f;
    }
    if (!b->move_still) {
        return 0.5f;
    }
    return 0.0f;
}

float calculate_feels_like(float base_temp, float wind_mps, const BehaviorInput* b) {
    const float feel = base_temp - wind_correction(wind_mps) + place_correction(b) + activity_correction(b);
    return feel;
}

static void update_minmax(float value, float* min_temp, float* max_temp) {
    if (value < *min_temp) {
        *min_temp = value;
    }
    if (value > *max_temp) {
        *max_temp = value;
    }
}

void calculate_distribution(float base_temp, float wind_mps, const BehaviorInput* b, Distribution* d) {
    float min_temp = FLT_MAX;
    float max_temp = -FLT_MAX;

    BehaviorInput outdoor = *b;
    outdoor.use_indoor = 0;
    d->outdoor_feel = calculate_feels_like(base_temp, wind_mps, &outdoor);

    BehaviorInput bridge = *b;
    bridge.use_indoor = 0;
    bridge.use_bridge = 1;
    d->bridge_feel = calculate_feels_like(base_temp, wind_mps, &bridge);

    d->indoor_feel = d->outdoor_feel + 3.0f;
    if (b->use_indoor) {
        d->indoor_feel += 0.5f;
    }

    if (b->time_cold_morning) {
        d->outdoor_feel -= 1.0f;
        d->bridge_feel -= 1.0f;
    }
    if (b->time_hot_noon) {
        d->outdoor_feel += 1.5f;
        d->indoor_feel += 1.0f;
    }
    if (b->time_cold_evening) {
        d->outdoor_feel -= 1.0f;
        d->bridge_feel -= 1.0f;
    }

    update_minmax(d->bridge_feel, &min_temp, &max_temp);
    update_minmax(d->outdoor_feel, &min_temp, &max_temp);
    update_minmax(d->indoor_feel, &min_temp, &max_temp);

    d->min_temp = min_temp;
    d->max_temp = max_temp;
    d->diff = d->max_temp - d->min_temp;
}

static void append_label(char* dst, size_t dst_size, const char* label) {
    if (dst[0] != '\0') {
        strncat(dst, " + ", dst_size - strlen(dst) - 1);
    }
    strncat(dst, label, dst_size - strlen(dst) - 1);
}

void analyze_strategy(const Distribution* d, float wind_mps, const BehaviorInput* b, StrategyResult* s) {
    s->strategy_main[0] = '\0';
    s->strategy_sub[0] = '\0';
    s->strategy_combined[0] = '\0';

    if (d->diff < 3.0f) {
        strncpy(s->strategy_main, "安定", sizeof(s->strategy_main) - 1);
        s->strategy_main[sizeof(s->strategy_main) - 1] = '\0';
    } else if (d->diff >= 6.0f) {
        strncpy(s->strategy_main, "調整前提", sizeof(s->strategy_main) - 1);
        s->strategy_main[sizeof(s->strategy_main) - 1] = '\0';
    } else {
        strncpy(s->strategy_main, "軽い調整", sizeof(s->strategy_main) - 1);
        s->strategy_main[sizeof(s->strategy_main) - 1] = '\0';
    }

    if (wind_mps >= 5.0f || b->use_windy_place || b->use_bridge) {
        append_label(s->strategy_sub, sizeof(s->strategy_sub), "防風対策");
    }
    if (d->min_temp < 13.0f) {
        append_label(s->strategy_sub, sizeof(s->strategy_sub), "寒さリスク");
    }
    if (d->max_temp > 24.0f) {
        append_label(s->strategy_sub, sizeof(s->strategy_sub), "暑さリスク");
    }
    if (b->move_walk_lots || b->move_bicycle) {
        append_label(s->strategy_sub, sizeof(s->strategy_sub), "軽装推奨");
    }

    if (s->strategy_sub[0] == '\0') {
        strncpy(s->strategy_sub, "標準対応", sizeof(s->strategy_sub) - 1);
        s->strategy_sub[sizeof(s->strategy_sub) - 1] = '\0';
    }

    s->strategy_combined[0] = '\0';
    strncat(s->strategy_combined, s->strategy_main, sizeof(s->strategy_combined) - 1);
    strncat(s->strategy_combined, " + ", sizeof(s->strategy_combined) - strlen(s->strategy_combined) - 1);
    strncat(s->strategy_combined, s->strategy_sub, sizeof(s->strategy_combined) - strlen(s->strategy_combined) - 1);

    if (wind_mps >= 8.0f) {
        strncpy(s->comment, "強風で体感が大きく低下。防風を最優先。", sizeof(s->comment) - 1);
    } else if (d->diff >= 6.0f) {
        strncpy(s->comment, "場所と時間で体感差が大きい。脱ぎ着前提で。", sizeof(s->comment) - 1);
    } else {
        strncpy(s->comment, "体感差は中程度。1枚追加で微調整。", sizeof(s->comment) - 1);
    }
    s->comment[sizeof(s->comment) - 1] = '\0';

    strncpy(s->guidance, "羽織りは必須 / 脱ぎ着できる構成 / 風を通しにくい素材を含める", sizeof(s->guidance) - 1);
    s->guidance[sizeof(s->guidance) - 1] = '\0';

    strncpy(s->ng_examples, "NG: 1枚で完結する服、温度変化を無視した固定装備", sizeof(s->ng_examples) - 1);
    s->ng_examples[sizeof(s->ng_examples) - 1] = '\0';

    strncpy(s->outfit_image, "例: 吸湿インナー + 長袖 + 軽量防風シェル", sizeof(s->outfit_image) - 1);
    s->outfit_image[sizeof(s->outfit_image) - 1] = '\0';
}

void generate_output(const WeatherInput* w, const Distribution* d, const StrategyResult* s) {
    printf("\n================ 服装戦略 結果 ================\n");
    printf("地域: %s\n", w->location);
    printf("日付: %s\n", w->date);
    printf("天気: %s\n", w->weather);

    printf("\n[1] 戦略\n");
    printf("  %s\n", s->strategy_combined);

    printf("\n[2] 体感温度の分布\n");
    printf("  橋:   %.1f℃\n", d->bridge_feel);
    printf("  外:   %.1f℃\n", d->outdoor_feel);
    printf("  室内: %.1f℃\n", d->indoor_feel);

    printf("\n[3] 状態\n");
    printf("  最低体感: %.1f℃\n", d->min_temp);
    printf("  最高体感: %.1f℃\n", d->max_temp);
    printf("  温度差:   %.1f℃\n", d->diff);

    printf("\n[4] 行動指針\n");
    printf("  %s\n", s->guidance);

    printf("\n[5] NG例\n");
    printf("  %s\n", s->ng_examples);

    printf("\n[6] 服装イメージ\n");
    printf("  %s\n", s->outfit_image);

    printf("\n補足: %s\n", s->comment);
    printf("==============================================\n");
}
