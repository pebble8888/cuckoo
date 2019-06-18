//
//  SiphashState.hpp
//
//  Created by pebble8888 on 2019/04/25.
//  Copyright © 2019 pebble8888. All rights reserved.
//

#pragma once

#include <stdio.h>
#include "SiphashKeys.hpp"

// 内部実装クラス
class SiphashState {
public:
    SiphashState(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);

    uint64_t xorLanes(void) const;
    void sipRound(void);
    void hash24(const uint64_t nonce);
    void compressWord(const uint64_t nonce, const uint8_t numOfRound);
    static uint64_t rotateLeft(const uint64_t x, const uint64_t bit);
private:
    uint64_t v0;
    uint64_t v1;
    uint64_t v2;
    uint64_t v3;
};
