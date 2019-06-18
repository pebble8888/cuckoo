// Cuckoo Cycle, a memory-hard proof-of-work
// Copyright (c) 2013-2016 John Tromp

#include "cuckoo.h"


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "cyclebase.h"
#include <set>
#include "cuckoo_xcode/cuckoo_xcode/cuckoo_ctx.hpp"

// -n means nonce
// simple19 -n 38
int main(int argc, char **argv)
{
    // arbitrary length of header hashed into siphash key
    static const int kHeaderLen = 80;
    char header[kHeaderLen] = {};
    int c;
    int easipct = 50;
    u32 base_nonce = 0;
    u32 range = 1;

    while ((c = getopt(argc, argv, "e:h:n:r:")) != -1) {
        switch (c) {
            case 'e':
                easipct = atoi(optarg);
                break;
            case 'h':
                memcpy(header, optarg, strlen(optarg));
                break;
            case 'n':
                base_nonce = atoi(optarg);
                break;
            case 'r':
                range = atoi(optarg);
                break;
        }
    }
    assert(easipct >= 0 && easipct <= 100);

    // PROOFSIZE: 42 
    // EDGEBITS: 19
    // header: ""
    // nonce: 38
    printf("Looking for %d-cycle on cuckoo%d(\"%s\",%d", PROOFSIZE, EDGEBITS+1, header, base_nonce);

    if (range > 1) {
        printf("-:%d", base_nonce + range -1);
    }

    // easipc: 50 %
    printf(") with %d%% edges, \n", easipct);

    const word_t easiness = easipct * (word_t)NNODES / 100;

    printf("easiness: %d\n", easiness);

    cuckoo_ctx ctx(easiness, NNODES);

    // range: 1
    printf("range: %d\n", range);

    for (u32 r = 0; r < range; r++) {
        const auto time0 = cuckoo_timestamp();

        const auto nonce = base_nonce + r;
        printf("nonce: %d\n", nonce);

        ctx.setHeaderNonce(header, sizeof(header), nonce);
        ctx.addEdges();
        ctx.invokeCycles();

        const auto time1 = cuckoo_timestamp();
        const auto timems = (time1 - time0) / 1000000;
        printf("Time: %lld ms\n", timems);
    }
    return 0;
}
