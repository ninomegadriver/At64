#ifdef __cplusplus
extern "C" {
#endif

// functions to communicate with the gc/n64 controller

void n64_send(const uint8_t* buff, uint8_t len,
    volatile uint8_t* modePort, volatile uint8_t* outPort, uint8_t bitMask)
    __attribute__((noinline));

uint8_t n64_get(uint8_t* buff, uint8_t len,
    volatile uint8_t* modePort, volatile uint8_t* outPort, volatile uint8_t * inPort, uint8_t bitMask)
    __attribute__((noinline));

#ifdef __cplusplus
}
#endif
