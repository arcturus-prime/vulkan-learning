struct FluidState {
    constexpr static size_t array_size = 50 * 50;

    unsigned char flow_x[array_size];
    unsigned char flow_y[array_size];
    unsigned char mass[array_size];

    void step();
};