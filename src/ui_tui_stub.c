#include <stdio.h>

#include "ui_tui.h"

int run_tui_app(void) {
    fprintf(stderr, "TUIビルドが無効です。`make tui` で taikan_tui をビルドしてください。\n");
    return 1;
}
