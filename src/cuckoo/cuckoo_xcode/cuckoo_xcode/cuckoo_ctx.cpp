//
//  cuckoo_ctx.cpp
//  cuckoo_xcode
//
//  Created by pebble8888 on 2019/05/01.
//  Copyright © 2019 pebble8888. All rights reserved.
//

#include "cuckoo_ctx.hpp"

cuckoo_ctx::cuckoo_ctx(
    word_t easyness,
    uint32_t num_nodes)
    : easiness_(easyness)
    , cyclebase_(Cyclebase(num_nodes))
{
}

void cuckoo_ctx::invokeCycles(void)
{
    cyclebase_.cycles();
}

// 
void cuckoo_ctx::setHeaderNonce(
    const char* headernonce,
    const uint32_t len,
    const uint32_t nonce)
{
    // place nonce at end
    ((u32 *)headernonce)[len/sizeof(u32)-1] = htole32(nonce);
    cuckoo_initialize_hasher(headernonce, len, siphash_);
    // 何をリセットする?
    cyclebase_.reset_count();
}

// エッジを追加
void cuckoo_ctx::addEdges(void)
{
    printf("NCUCKOO: %d\n", NCUCKOO); // 1048576
    printf("easiness_: %d\n", easiness_); // 524288
    // 半分(50%)を埋める
    // 0, 1, 2, 3, ..., 524288-1に対して行う
    for (word_t nonce = 0; nonce < easiness_; nonce++) {
        const word_t u_bare = cuckoo_sipnode(siphash_, nonce, 0);
        const word_t v_bare = cuckoo_sipnode(siphash_, nonce, 1);
        cyclebase_.add_edge(u_bare, v_bare);
    }
}

