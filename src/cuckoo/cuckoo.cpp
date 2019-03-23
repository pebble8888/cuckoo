//
//  cuckoo.cpp
//  cuckoo_xcode
//
//  Created by pebble8888 on 2019/03/23.
//  Copyright © 2019 pebble8888. All rights reserved.
//

#include "cuckoo.h"

int verify(word_t edges[PROOFSIZE], Siphash_keys *keys)
{
    word_t uvs[2*PROOFSIZE];
    word_t xor0 = 0, xor1 = 0;
    for (u32 n = 0; n < PROOFSIZE; n++) {
        if (edges[n] > EDGEMASK) {
            return POW_TOO_BIG;
        }
        if (n && edges[n] <= edges[n-1]) {
            return POW_TOO_SMALL;
        }
        xor0 ^= uvs[2*n  ] = sipnode(keys, edges[n], 0);
        xor1 ^= uvs[2*n+1] = sipnode(keys, edges[n], 1);
    }
    if (xor0|xor1) {              // optional check for obviously bad proofs
        return POW_NON_MATCHING;
    }
    u32 n = 0;
    u32 i = 0;
    u32 j;
    do {                        // follow cycle
        for (u32 k = j = i; (k = (k+2) % (2*PROOFSIZE)) != i; ) {
            if (uvs[k] == uvs[i]) { // find other edge endpoint identical to one at i
                if (j != i) {           // already found one before
                    return POW_BRANCH;
                }
                j = k;
            }
        }
        if (j == i) {
            return POW_DEAD_END;  // no matching endpoint
        }
        i = j^1;
        n++;
    } while (i != 0);           // must cycle back to start or we would have found branch
    return n == PROOFSIZE ? POW_OK : POW_SHORT_CYCLE;
}

void setheader(const char *header, const u32 headerlen, Siphash_keys *keys) {
    char hdrkey[32];
    // SHA256((unsigned char *)header, headerlen, (unsigned char *)hdrkey);
    blake2b((void *)hdrkey, sizeof(hdrkey), (const void *)header, headerlen, 0, 0);
#ifdef SIPHASH_COMPAT
    u64 *k = (u64 *)hdrkey;
    u64 k0 = k[0];
    u64 k1 = k[1];
    printf("k0 k1 %lx %lx\n", k0, k1);
    k[0] = k0 ^ 0x736f6d6570736575ULL;
    k[1] = k1 ^ 0x646f72616e646f6dULL;
    k[2] = k0 ^ 0x6c7967656e657261ULL;
    k[3] = k1 ^ 0x7465646279746573ULL;
#endif
    keys->setkeys(hdrkey);
}

word_t sipnode_(Siphash_keys *keys, word_t edge, u32 uorv) {
    return sipnode(keys, edge, uorv) << 1 | uorv;
}

word_t sipnode(Siphash_keys *keys, word_t edge, u32 uorv) {
    return keys->siphash24(2*edge + uorv) & EDGEMASK;
}

u64 timestamp() {
    using namespace std::chrono;
    high_resolution_clock::time_point now = high_resolution_clock::now();
    auto dn = now.time_since_epoch();
    return dn.count();
}

void print_log(const char *fmt, ...) {
    if (SQUASH_OUTPUT) return;
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
