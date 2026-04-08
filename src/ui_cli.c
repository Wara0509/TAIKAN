#include "ui_cli.h"

#include <stdio.h>
#include <string.h>

#include "logic.h"

static int ask_choice(const char* prompt, int min, int max) {
    int v = 0;
    int ok = 0;

    while (!ok) {
        printf("%s", prompt);
        if (scanf("%d", &v) == 1 && v >= min && v <= max) {
            ok = 1;
        } else {
            printf("入力エラー: %d〜%d の数値を入力してください。\n", min, max);
        }
        while (getchar() != '\n') {
            /* flush */
        }
    }

    return v;
}

void collect_weather_input(WeatherInput* w) {
    printf("地域名を入力してください: ");
    scanf("%63s", w->location);

    printf("日付を入力してください (例: 2026-04-08): ");
    scanf("%63s", w->date);

    printf("天気を入力してください (例: Cloudy): ");
    scanf("%63s", w->weather);

    printf("気温(℃)を入力してください: ");
    scanf("%f", &w->temp_c);

    printf("風速(m/s)を入力してください: ");
    scanf("%f", &w->wind_mps);

    while (getchar() != '\n') {
        /* flush */
    }
}

void run_quick_mode(const WeatherInput* w) {
    BehaviorInput b = {0};
    Distribution d;
    StrategyResult s;
    float feels_like;

    /* 即決モード: 行動入力なしでも最短で戦略を出す */
    calculate_distribution(w->temp_c, w->wind_mps, &b, &d);
    analyze_strategy(&d, w->wind_mps, &b, &s);
    feels_like = calculate_feels_like(w->temp_c, w->wind_mps, &b);

    printf("\n========== 即決モード =========="
           "\n地域: %s"
           "\n日付: %s"
           "\n天気: %s"
           "\n気温: %.1f℃"
           "\n体感温度: %.1f℃"
           "\n体感差: %.1f℃"
           "\n\n【戦略】%s"
           "\n補足: %s"
           "\n===============================\n",
           w->location,
           w->date,
           w->weather,
           w->temp_c,
           feels_like,
           feels_like - w->temp_c,
           s.strategy_combined,
           s.comment);
}

void run_precise_mode(const WeatherInput* w, BehaviorInput* b) {
    Distribution d;
    StrategyResult s;

    memset(b, 0, sizeof(*b));

    printf("\n---- 精密モード: STEP1 環境 ----\n");
    printf("1) 橋を通る\n");
    printf("2) 風が強い場所\n");
    printf("3) 吹き抜け\n");
    printf("4) 高所\n");
    printf("5) 日陰が多い\n");
    printf("6) 日向が多い\n");
    printf("7) 室内が多い\n");
    printf("8) 特になし\n");
    {
        const int env = ask_choice("環境を1つ選択してください [1-8]: ", 1, 8);
        b->use_bridge = (env == 1);
        b->use_windy_place = (env == 2);
        b->use_atrium = (env == 3);
        b->use_high_place = (env == 4);
        b->use_shade = (env == 5);
        b->use_sunny = (env == 6);
        b->use_indoor = (env == 7);
    }

    printf("\n---- 精密モード: STEP2 移動 ----\n");
    printf("1) 徒歩多い\n");
    printf("2) 自転車\n");
    printf("3) ほぼ動かない\n");
    printf("4) 普通\n");
    {
        const int move = ask_choice("移動タイプを選択してください [1-4]: ", 1, 4);
        b->move_walk_lots = (move == 1);
        b->move_bicycle = (move == 2);
        b->move_still = (move == 3);
    }

    printf("\n---- 精密モード: STEP3 時間変化 ----\n");
    printf("1) 朝寒い\n");
    printf("2) 昼暑い\n");
    printf("3) 夕方寒い\n");
    printf("4) 変化少ない\n");
    {
        const int time = ask_choice("時間変化を選択してください [1-4]: ", 1, 4);
        b->time_cold_morning = (time == 1);
        b->time_hot_noon = (time == 2);
        b->time_cold_evening = (time == 3);
    }

    calculate_distribution(w->temp_c, w->wind_mps, b, &d);
    analyze_strategy(&d, w->wind_mps, b, &s);
    generate_output(w, &d, &s);
}
