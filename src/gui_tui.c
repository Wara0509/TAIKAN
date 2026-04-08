#include "ui_tui.h"

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logic.h"
#include "model.h"

static int menu_select(const char* title, const char* const items[], int n) {
    int current = 0;
    int ch;

    keypad(stdscr, TRUE);
    while (1) {
        clear();
        mvprintw(1, 2, "%s", title);
        mvprintw(2, 2, "↑↓で選択、Enterで決定");

        for (int i = 0; i < n; i++) {
            if (i == current) {
                attron(A_REVERSE);
            }
            mvprintw(4 + i, 4, "%s", items[i]);
            if (i == current) {
                attroff(A_REVERSE);
            }
        }

        refresh();
        ch = getch();
        if (ch == KEY_UP && current > 0) {
            current--;
        } else if (ch == KEY_DOWN && current < n - 1) {
            current++;
        } else if (ch == '\n' || ch == KEY_ENTER || ch == 10 || ch == 13) {
            return current;
        }
    }
}

static void get_text_input(int y, const char* label, char* out, int size, const char* default_value) {
    mvprintw(y, 2, "%s", label);
    mvprintw(y, 22, "[%s]", default_value);
    move(y, 2 + (int)strlen(label) + 1);
    clrtoeol();
    mvprintw(y, 2, "%s", label);

    echo();
    curs_set(1);
    getnstr(out, size - 1);
    noecho();
    curs_set(0);

    if (out[0] == '\0') {
        strncpy(out, default_value, size - 1);
        out[size - 1] = '\0';
    }
}

static float get_float_input(int y, const char* label, float default_value) {
    char buf[32];
    float v = default_value;

    mvprintw(y, 2, "%s (default %.1f): ", label, default_value);
    echo();
    curs_set(1);
    getnstr(buf, (int)sizeof(buf) - 1);
    noecho();
    curs_set(0);

    if (buf[0] != '\0') {
        v = strtof(buf, NULL);
    }
    return v;
}

static void collect_weather_tui(WeatherInput* w) {
    clear();
    mvprintw(1, 2, "[天気入力] 空欄Enterでデフォルト採用");

    get_text_input(3, "地域:", w->location, TEXT_SMALL, "Tokyo");
    get_text_input(5, "日付:", w->date, TEXT_SMALL, "2026-04-08");
    get_text_input(7, "天気:", w->weather, TEXT_SMALL, "Cloudy");
    w->temp_c = get_float_input(9, "気温(℃)", 18.0f);
    w->wind_mps = get_float_input(11, "風速(m/s)", 4.0f);

    mvprintw(13, 2, "入力完了。何かキーを押してください...");
    refresh();
    getch();
}

static void run_quick_tui(const WeatherInput* w) {
    BehaviorInput b = {0};
    Distribution d;
    StrategyResult s;
    const float feels = calculate_feels_like(w->temp_c, w->wind_mps, &b);

    calculate_distribution(w->temp_c, w->wind_mps, &b, &d);
    analyze_strategy(&d, w->wind_mps, &b, &s);

    clear();
    mvprintw(1, 2, "[即決モード]");
    mvprintw(3, 2, "地域: %s", w->location);
    mvprintw(4, 2, "日付: %s", w->date);
    mvprintw(5, 2, "天気: %s", w->weather);
    mvprintw(6, 2, "気温: %.1f℃", w->temp_c);
    mvprintw(7, 2, "体感: %.1f℃", feels);
    mvprintw(8, 2, "体感差: %.1f℃", feels - w->temp_c);

    attron(A_BOLD);
    mvprintw(10, 2, "戦略: %s", s.strategy_combined);
    attroff(A_BOLD);

    mvprintw(12, 2, "補足: %s", s.comment);
    mvprintw(14, 2, "何かキーでメニューへ戻る");
    refresh();
    getch();
}

static void run_precise_tui(const WeatherInput* w) {
    BehaviorInput b = {0};
    Distribution d;
    StrategyResult s;

    const char* const env_items[] = {
        "橋を通る", "風が強い場所", "吹き抜け", "高所", "日陰が多い", "日向が多い", "室内が多い", "特になし"};
    const char* const move_items[] = {
        "徒歩多い", "自転車", "ほぼ動かない", "普通"};
    const char* const time_items[] = {
        "朝寒い", "昼暑い", "夕方寒い", "変化少ない"};

    int env = menu_select("[精密モード STEP1] 環境", env_items, 8);
    int move = menu_select("[精密モード STEP2] 移動", move_items, 4);
    int time = menu_select("[精密モード STEP3] 時間変化", time_items, 4);

    b.use_bridge = (env == 0);
    b.use_windy_place = (env == 1);
    b.use_atrium = (env == 2);
    b.use_high_place = (env == 3);
    b.use_shade = (env == 4);
    b.use_sunny = (env == 5);
    b.use_indoor = (env == 6);

    b.move_walk_lots = (move == 0);
    b.move_bicycle = (move == 1);
    b.move_still = (move == 2);

    b.time_cold_morning = (time == 0);
    b.time_hot_noon = (time == 1);
    b.time_cold_evening = (time == 2);

    calculate_distribution(w->temp_c, w->wind_mps, &b, &d);
    analyze_strategy(&d, w->wind_mps, &b, &s);

    clear();
    mvprintw(1, 2, "[結果]");
    attron(A_BOLD);
    mvprintw(3, 2, "戦略: %s", s.strategy_combined);
    attroff(A_BOLD);

    mvprintw(5, 2, "分布: 橋 %.1f℃ / 外 %.1f℃ / 室内 %.1f℃", d.bridge_feel, d.outdoor_feel, d.indoor_feel);
    mvprintw(6, 2, "状態: min %.1f℃ / max %.1f℃ / diff %.1f℃", d.min_temp, d.max_temp, d.diff);

    mvprintw(8, 2, "行動指針: %s", s.guidance);
    mvprintw(10, 2, "NG例: %s", s.ng_examples);
    mvprintw(12, 2, "服装イメージ: %s", s.outfit_image);
    mvprintw(14, 2, "補足: %s", s.comment);

    mvprintw(16, 2, "何かキーでメニューへ戻る");
    refresh();
    getch();
}

int run_tui_app(void) {
    const char* const menu_items[] = {
        "即決モード（数秒で戦略）",
        "精密モード（3ステップ入力）",
        "終了"};

    WeatherInput w;

    initscr();
    cbreak();
    noecho();
    curs_set(0);

    while (1) {
        collect_weather_tui(&w);
        while (1) {
            int choice = menu_select("TAIKAN: 服装意思決定を代替する戦略アプリ", menu_items, 3);
            if (choice == 0) {
                run_quick_tui(&w);
            } else if (choice == 1) {
                run_precise_tui(&w);
            } else {
                endwin();
                return 0;
            }
        }
    }

    endwin();
    return 0;
}
