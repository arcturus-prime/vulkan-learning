#include "graphics.hpp"

int main() {
    GraphicsContext context;
    context.Init();

    while (!context.WantsToTerminate()) {
        context.Step();
    }

    return 0;
}