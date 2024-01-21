void ms_delay(unsigned int ms) {
    ms *= 4U;
    while (ms-- > 0U) {
        volatile unsigned int x = 400U;
        while (x-- > 0U) { __asm("nop"); }
    }
}

// void ten_microseconds_delay() {
//     // volatile unsigned int x = 16U;
//     volatile unsigned int x = 20U;
//     while (x-- > 0U) { __asm("nop"); }
// }
