//
//  cuckoo_ctx.hpp
//  cuckoo_xcode
//
//  Created by pebble8888 on 2019/05/01.
//  Copyright © 2019 pebble8888. All rights reserved.
//

#ifndef cuckoo_ctx_hpp
#define cuckoo_ctx_hpp

#include <stdio.h>
#include <stdint.h>
#include "cuckoo.h"
#include "cyclebase.h"

class SiphashKeys;

// assume EDGEBITS < 31
#define NNODES (2 * NEDGES)
#define NCUCKOO NNODES


class cuckoo_ctx {
public:
    // easyness: 524288
    // num_nodes: 1048576
    cuckoo_ctx(word_t easyness, uint32_t num_nodes);

    void invokeCycles(void);
    void setHeaderNonce(const char* headernonce, const uint32_t len, const uint32_t nonce);
    void addEdges(void);
private:
    Cyclebase cyclebase_;
    Siphash siphash_;
    word_t easiness_; ///< 524288
};

#endif /* cuckoo_ctx_hpp */
