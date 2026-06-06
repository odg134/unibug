#include <intrin.h>
#include <array>
#include <cstdio>

extern "C" void foobar( void* dst, const void* src );

bool is_avx_available() {
    int regs[4] = {};
    __cpuid(regs, 1);
    
    //
    // OSXSAVE + AVX, then XMM/YMM state enabled in XCR0.
    //

    const bool osxsave = (regs[2] & (1 << 27)) != 0;
    const bool avx = (regs[2] & (1 << 28)) != 0;
    if (!osxsave || !avx) {
        return false;
    }

    const unsigned long long xcr0 = _xgetbv(0);
    return (xcr0 & 0x6) == 0x6;
}

int main() {
    std::puts("hello world");

    if (!is_avx_available()) {
        std::puts("AVX is not available.");
        return 2;
    }

    alignas(32) std::array<char, 32> packet = {
        'h', 'e', 'l', 'l', 'o', 'w', 'o', 'r',
        'l', 'd',
        0, 0, 0, 0, 0, 0, 0, 0
    };

    alignas(32) std::array<char, 32> passed = {};

    //
    // This will invoke our ASM function that
    // should pass the 32 byte packet with AVX instructions.
    //
    foobar(passed.data(), packet.data());

    const bool ok = packet == passed;
    std::printf("passed data: %s\n", ok ? passed.data() : "<failed>");

    return ok ? 0 : 1;
}
