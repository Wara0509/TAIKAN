#include <stdio.h>
#include <string.h>

#include "ui_cli.h"
#include "ui_tui.h"

int main(int argc, char** argv) {
    WeatherInput weather;
    BehaviorInput behavior;
    int mode;

    if (argc > 1 && strcmp(argv[1], "--tui") == 0) {
        return run_tui_app();
    }

    printf("========================================\n");
    printf(" TAIKAN - 服装戦略提案アプリ (CLI版)\n");
    printf(" これは天気表示アプリではなく、\n");
    printf(" 服装の意思決定を代替するためのアプリです。\n");
    printf(" GUI風操作版は `./taikan_tui` または `./taikan --tui` を使用\n");
    printf("========================================\n\n");

    collect_weather_input(&weather);

    printf("\nモードを選択してください:\n");
    printf("1) 即決モード（数秒で戦略）\n");
    printf("2) 精密モード（3ステップ入力）\n");
    printf("> ");

    if (scanf("%d", &mode) != 1) {
        printf("入力エラー: 数値を入力してください。\n");
        return 1;
    }

    if (mode == 1) {
        run_quick_mode(&weather);
    } else if (mode == 2) {
        run_precise_mode(&weather, &behavior);
    } else {
        printf("未対応のモードです。1 または 2 を選択してください。\n");
        return 1;
    }

    return 0;
}
