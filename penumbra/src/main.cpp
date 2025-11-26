#include <iostream>
#include "main.h"

int main() {
    Viewport viewport(960, 540);
    auto& buffer = viewport.GetWindowBuffer();
    
    for (size_t i = 0; i < buffer.size(); i += 3) {
        buffer[i + 0] = 255;  // R
        buffer[i + 1] = 0;    // G
        buffer[i + 2] = 255;    // B
    }

    while (!viewport.ShouldClose()) {
        viewport.PollEvents();
        viewport.ShowViewport();
    }
    return 0;
}