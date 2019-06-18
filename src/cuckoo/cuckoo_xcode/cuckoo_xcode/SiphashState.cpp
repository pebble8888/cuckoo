//
//  SiphashState.cpp
//
//  Created by pebble8888 on 2019/04/25.
//  Copyright © 2019 pebble8888. All rights reserved.
//

#include "SiphashState.hpp"

SiphashState::SiphashState(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3)
    : v0(v0)
    , v1(v1)
    , v2(v2)
    , v3(v3)
{
}

uint64_t SiphashState::xorLanes(void) const
{
    return (v0 ^ v1) ^ (v2 ^ v3);
}

uint64_t SiphashState::rotateLeft(const uint64_t x, const uint64_t bit)
{
    return (x << bit) | (x >> (64 - bit));
}

void SiphashState::sipRound(void)
{
    v0 += v1;
    v1 = rotateLeft(v1, 13);
    v1 ^= v0;
    v0 = rotateLeft(v0, 32);
    v2 += v3;
    v3 = rotateLeft(v3, 16);
    v3 ^= v2;
    v0 += v3;
    v3 = rotateLeft(v3, 21);
    v3 ^= v0;
    v2 += v1;
    v1 = rotateLeft(v1, 17);
    v1 ^= v2;
    v2 = rotateLeft(v2, 32);
}

void SiphashState::compressWord(const uint64_t nonce, const uint8_t numOfRound)
{
    v3 ^= nonce;
    for (int i = 0; i < numOfRound; ++i) {
        sipRound();
    }
    v0 ^= nonce;
}

void SiphashState::hash24(const uint64_t nonce)
{
    const uint8_t numOfCompressionRound = 2;
    compressWord(nonce, numOfCompressionRound);

    // Finalization
    v2 ^= 0xff;
    const uint8_t numOfFinalizationRound = 4;
    for (int i = 0; i < numOfFinalizationRound; ++i) {
        sipRound();
    }
}
