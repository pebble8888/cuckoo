#pragma once

#include <stdint.h>
#include <string>
#include "portable_endian.h"    // for htole32/64

// 外部で使うクラス
// generalize siphash by using a quadruple of 64-bit keys,
class Siphash {
public:
    // 入力は128bit = 64 bit * 2
    void setkeys(const char *keybuf);
    // @param nonce キー
    uint64_t siphash24(const uint64_t nonce) const;

    std::string description(void) const;
private:
    uint64_t k0;
    uint64_t k1;
    uint64_t k2;
    uint64_t k3;
};
