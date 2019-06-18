//
//  cuckoo.cpp
//  cuckoo_xcode
//
//  Created by pebble8888 on 2019/03/23.
//  Copyright © 2019 pebble8888. All rights reserved.
//

#include "cuckoo.h"

/**
 @brief ProofOfWork をベリファイする
 @param edges エッジ
 @param num_edges エッジの数 多分42
 @param siphash ハッシュ生成器
 */
VerifyCode cuckoo_verifyPOW(word_t* edges, uint32_t num_edges, const Siphash& siphash)
{
    word_t uvs[2*PROOFSIZE];
    word_t xor0 = 0; // for optional check
    word_t xor1 = 0; // for optional check
    // PROOFSIZE = 42
    // EDGEBITS = 19
    // NEDGES = 1 << 19 = 524288
    for (u32 n = 0; n < PROOFSIZE; ++n) {
        if (edges[n] > EDGEMASK) {
            // エッジに入っている値が大きすぎる
            return VerifyCode::POW_TOO_BIG;
        }
        if (n > 0 && edges[n-1] >= edges[n]) {
            // エッジの値は昇順でなければならない
            return VerifyCode::POW_TOO_SMALL;
        }
        uvs[2*n] = cuckoo_sipnode(siphash, edges[n], 0);
        xor0 ^= uvs[2*n];

        uvs[2*n+1] = cuckoo_sipnode(siphash, edges[n], 1);
        xor1 ^= uvs[2*n+1];
    }
    if ((xor0|xor1) != 0) {
        // optional check for obviously bad proofs
        // --> ??? 後回しというか無視してよい
        return VerifyCode::POW_NON_MATCHING;
    }
    uint32_t n = 0;
    uint32_t i = 0;
    while (true) {
        // follow cycle
        uint32_t j = i;
        for (uint32_t k = i; ;) {
            k = (k+2) % (2 * PROOFSIZE);
            if (k == i) {
                // 1周した
                break;
            }
            if (uvs[k] == uvs[i]) {
                // find other edge endpoint identical to one at i
                if (j != i) {
                    // already found one before
                    return VerifyCode::POW_BRANCH;
                }
                j = k;
            }
        }
        if (j == i) {
            // no matching endpoint
            return VerifyCode::POW_DEAD_END;
        }
        // 下位の1ビットを反転させる
        i = j^1;
        n++;
        // must cycle back to start or we would have found branch
        if (i == 0) {
            break;
        }
    }
    if (n == PROOFSIZE) {
        return VerifyCode::POW_OK;
    } else {
        return VerifyCode::POW_SHORT_CYCLE;
    }
}

/**
 * @brief ハッシュ生成器を初期化する
 * @param header ヘッダの先頭ポインタ
 * @param headerlen ヘッダの長さ
 * @param siphash ハッシュ生成器
 */
void cuckoo_initialize_hasher(const char *header, const u32 headerlen, Siphash& siphash)
{
    char hdrkey[32]; // 8bit * 32 = 64bit * 4
    // blake2bの第２引数は出力サイズ, 64バイト以下1バイト以上であればOK
    blake2b((void *)hdrkey, sizeof(hdrkey), (const void *)header, headerlen, 0, 0);
    // ここではSIPHASHのinitializationロジックを使わずにblake2b()ハッシュの結果をInitializationとして使う.
    // setkey()の引数は64bit * 4 = 32バイトの長さが必要である
    siphash.setkeys(hdrkey);
}

/**
 * @brief エッジの値を2倍してuorvの値を加算したもののハッシュ値をエッジマスクしたものを返す
 * @param siphash : ハッシュ生成器
 * @param edge : エッジの値
 * @param uorv : 0 or 1
 */
word_t cuckoo_sipnode(const Siphash& siphash, word_t edge, u32 uorv)
{
    assert(uorv == 0 || uorv == 1);
    return siphash.siphash24(2*edge + uorv) & EDGEMASK;
}

u64 cuckoo_timestamp(void)
{
    using namespace std::chrono;
    const auto now = high_resolution_clock::now();
    const auto dn = now.time_since_epoch();
    return dn.count();
}

void cuckoo_print_log(const char *fmt, ...)
{
    //if (SQUASH_OUTPUT) return;
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

