#include "graphics.hpp"

int main() {
    GraphicsContext context;

    while (!context.WantsToTerminate()) {
        context.Step();
    }

    return 0;
}