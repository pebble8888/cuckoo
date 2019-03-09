// Cuckoo Cycle, a memory-hard proof-of-work
// Copyright (c) 2013-2016 John Tromp

#include "cuckoo.h"
#include <inttypes.h> // for SCNx64 macro
#include <stdio.h>    // printf/scanf
#include <stdlib.h>   // exit
#include <unistd.h>   // getopt
#include <assert.h>   // d'uh

// arbitrary length of header hashed into siphash key
// uint32_t * 20 
#define HEADERLEN 80

int main(int argc, char **argv) {
    const char *header = "";
    int nonce = 0;
    int c;
    while ((c = getopt (argc, argv, "h:n:")) != -1) {
        switch (c) {
            case 'h':
                header = optarg;
                break;
            case 'n':
                nonce = atoi(optarg);
                break;
        }
    }
    // 80バイトの headernonce
    char headernonce[HEADERLEN];
    const u32 hdrlen = strlen(header);
    // 引数で渡されたヘッダを先頭に詰める
    memcpy(headernonce, header, hdrlen);
    // 残りを0で埋める
    memset(headernonce+hdrlen, 0, sizeof(headernonce)-hdrlen);
    // 引数で渡されたnonceを
    // 80/4-1 = 20-1 = 19
    // uint32_t 領域の20番目の位置にnonceをセットする
    ((u32 *)headernonce)[HEADERLEN/sizeof(u32)-1] = htole32(nonce);
    Siphash_keys keys;
    setheader(headernonce, sizeof(headernonce), &keys);
    printf("Verifying size %d proof for cuckoo%d(\"%s\",%d)\n",
            PROOFSIZE, EDGEBITS+1, header, nonce);
    for (int nsols = 0; scanf(" Solution") == 0; nsols++) {
        word_t nonces[PROOFSIZE];
        for (int n = 0; n < PROOFSIZE; n++) {
            uint64_t nonce;
            int nscan = scanf(" %" SCNx64, &nonce);
            assert(nscan == 1);
            nonces[n] = nonce;
        }
        int pow_rc = verify(nonces, &keys);
        if (pow_rc == POW_OK) {
            printf("Verified with cyclehash ");
            unsigned char cyclehash[32];
            blake2b((void *)cyclehash, sizeof(cyclehash), (const void *)nonces, sizeof(nonces), 0, 0);
            for (int i=0; i<32; i++)
                printf("%02x", cyclehash[i]);
            printf("\n");
        } else {
            printf("FAILED due to %s\n", errstr[pow_rc]);
        }
    }
    return 0;
}

