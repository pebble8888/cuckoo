// Cuckoo Cycle, a memory-hard proof-of-work
// Copyright (c) 2013-2016 John Tromp

#include "cuckoo.h"

// assume EDGEBITS < 31
#define NNODES (2 * NEDGES)
#define NCUCKOO NNODES


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "cyclebase.hpp"
#include <set>

typedef unsigned char u8;

class cuckoo_ctx {
private:
    Siphash_keys sip_keys_;
    word_t       easiness_;
    Cyclebase    cb_ = Cyclebase(NCUCKOO);

public:
    void cycles(void) {
        cb_.cycles();
    }

    const Cyclebase& cb(void) const {
        return cb_;
    }

    const Siphash_keys& sip_keys(void) const {
        return sip_keys_;
    }

    cuckoo_ctx(
        const char* header, 
        const u32 headerlen,
        const u32 nonce, 
        word_t easy_ness)
    {
        easiness_ = easy_ness;
    }

    word_t bytes()
    {
        return (word_t)(1+NNODES) * sizeof(word_t);
    }

    void set_header_nonce(char* const headernonce, const u32 len, const u32 nonce)
    {
        ((u32 *)headernonce)[len/sizeof(u32)-1] = htole32(nonce); // place nonce at end
        setheader(headernonce, len, &sip_keys_);
        // 何をリセットする?
        cb_.reset_count();
    }

    // エッジを追加
    void cycle_base()
    {
        printf("NCUCKOO: %d\n", NCUCKOO);
        printf("easiness_: %d\n", easiness_);
        // 半分(50%)を埋める
        for (word_t nonce = 0; nonce < easiness_; nonce++) {
            word_t u = sipnode(&sip_keys_, nonce, 0);
            word_t v = sipnode(&sip_keys_, nonce, 1);
            cb_.add_edge(u, v);
        }
    }
};

// arbitrary length of header hashed into siphash key
#define HEADERLEN 80

// -n means nonce
// simple19 -n 38
int main(int argc, char **argv)
{
    char header[HEADERLEN];
    memset(header, 0, HEADERLEN);
    int c;
    int easipct = 50;
    u32 nonce = 0;
    u32 range = 1;
    u64 time0;
    u64 time1;
    u32 timems;

    while ((c = getopt(argc, argv, "e:h:n:r:")) != -1) {
        switch (c) {
            case 'e':
                easipct = atoi(optarg);
                break;
            case 'h':
                memcpy(header, optarg, strlen(optarg));
                break;
            case 'n':
                nonce = atoi(optarg);
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
    printf("Looking for %d-cycle on cuckoo%d(\"%s\",%d", PROOFSIZE, EDGEBITS+1, header, nonce);

    if (range > 1) {
        printf("-:%d", nonce+range-1);
    }

    // easipc: 50 %
    printf(") with %d%% edges, ", easipct);

    word_t easiness = easipct * (word_t)NNODES / 100;
    cuckoo_ctx ctx(header, sizeof(header), nonce, easiness);
    word_t bytes = ctx.bytes();
    int unit;
    for (unit = 0; bytes >= 10240; bytes >>= 10, unit++) {
        ;
    }
    printf("using %d%cB memory at %llx.\n",
            bytes, " KMGT"[unit], (uint64_t)ctx.cb().cuckoo_address());

    printf("range: %d\n", range);
    for (u32 r = 0; r < range; r++) {
        time0 = timestamp();
        ctx.set_header_nonce(header, sizeof(header), nonce + r);

        printf("nonce %d k0 k1 k2 k3 %llx %llx %llx %llx\n", nonce+r,
                ctx.sip_keys().k0,
                ctx.sip_keys().k1,
                ctx.sip_keys().k2,
                ctx.sip_keys().k3);

        // エッジを追加
        ctx.cycle_base();

        //
        ctx.cycles();
        time1 = timestamp(); timems = (time1 - time0) / 1000000;

        printf("Time: %d ms\n", timems);
    }
}

