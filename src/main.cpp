#include "UI.h"

int main(int, char**) {
    UI app;
    if (!app.init()) {
        printf("Failed to initialize.\n");
        return 1;
    }
    app.run();
    return 0;
}
