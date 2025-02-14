#include <iostream>
#include <exception>

#include "graphics.hpp"

int main() {
    try {
        GraphicsContext context;

        while (!context.WantsToTerminate()) {
            context.Step();
        }
    } catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    return 0;
}