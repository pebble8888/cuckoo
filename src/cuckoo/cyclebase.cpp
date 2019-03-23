#include "cyclebase.hpp"


void Cyclebase::alloc(void) {
    cuckoos_ = new word_t[n_cuckoo_];
    memset(cuckoos_, 0, sizeof(word_t) * n_cuckoo_);
    path_counts_ = new u32[n_cuckoo_];
    memset(path_counts_, 0, sizeof(u32) * n_cuckoo_);
}

void Cyclebase::freemem(void) { // not a destructor, as memory may have been allocated elsewhere, bypassing alloc()
    delete [] cuckoos_;
    delete [] path_counts_;
}

// 何をリセットする? 
void Cyclebase::reset_count(void) {
    for (auto i = 0; i < n_cuckoo_; ++i) {
        cuckoos_[i] = -1; // for prevcycle nil
        path_counts_[i] = 0;
    }
    n_cycles_ = 0;
}

void Cyclebase::add_edge(const u32 u0, const u32 v0) {
    u32 u_pathes[kMaxPathLen]; // work領域
    u32 v_pathes[kMaxPathLen]; // work領域
    const u32 u = u0 << 1; // u0の値を2倍する
    const u32 v = (v0 << 1) | 1; // v0の値を2倍して1を足す
    unsigned int nu = getpath(u, u_pathes);
    unsigned int nv = getpath(v, v_pathes);
    if (u_pathes[nu] == v_pathes[nv]) {
        // cuckoo配列の最後の値が一致している: found cycle
        // 要求される長さとは限らない
        
        // このとき path_counts_ は増加させない(なぜか?)

        join_path(u_pathes, nu, v_pathes, nv);
        const int len = nu + nv + 1;

        printf("% 4d-cycle found\n", len);

        // サイクルの位置を保存する
        cycle_edges_[n_cycles_].u = u;
        cycle_edges_[n_cycles_].v = v;
        // サイクルの長さを保存する
        cycle_lengths_[n_cycles_] = len;
        // サイクル数を増加
        n_cycles_ += 1;
        if (len == PROOFSIZE) {
            // 要求サイズを満たしたサイクルである
            print_solution(u_pathes, nu, v_pathes, nv);
        }
        assert(n_cycles_ < MAXCYCLES);
    } else if (nu < nv) {
        // u側サイクルの方が少ない
        // 少ない側(u)のパスを伸ばす
        const auto i = u_pathes[nu];
        path_counts_[i] += 1;
        // 少ない側(u)のカッコーを更新する
        for (int k = nu; k > 0; --k) {
            const auto j = u_pathes[k];
            if (cuckoos_[j] != -1) {
                printf("cuckoo!\n");
            }
            cuckoos_[j] = u_pathes[k-1];
            printf("cuckoos_[%d]: %d\n", j, cuckoos_[j]);				
        }
        if (cuckoos_[u] != -1) {
            printf("cuckoo!\n");
        }
        cuckoos_[u] = v;
        printf("cuckoos_[%d]: %d\n", u, cuckoos_[u]);							
    } else {
        // v側サイクルの方が少ないもしくはサイクルなし
        // 少ない側(v)のパスを伸ばす
        const auto i = v_pathes[nv];
        path_counts_[i] += 1;
        // 少ない側(v)のカッコーを更新する
        for (int k = nv; k > 0; --k) {
            const auto j = v_pathes[k];
            if (cuckoos_[j] != -1) {
                printf("cukcoo!\n");
            }				
            cuckoos_[j] = v_pathes[k-1];
            printf("cuckoos_[%d]: %d\n", j, cuckoos_[j]);				
        }
        if (cuckoos_[v] != -1) {
            printf("cuckoo!\n");
        }
        cuckoos_[v] = u;
        printf("cuckoos_[%d]: %d\n", v, cuckoos_[v]);							
    }
}

void Cyclebase::cycles(void) {
    u32 u_pathes[kMaxPathLen];
    u32 v_pathes[kMaxPathLen];
    word_t us2[kMaxPathLen];
    word_t vs2[kMaxPathLen];
    for (int i = 0; i < n_cycles_; i++) {
        const word_t u3 = cycle_edges_[i].u;
        const word_t v3 = cycle_edges_[i].v;
        unsigned int nu = getpath(u3, u_pathes);
        unsigned int nv = getpath(v3, v_pathes);
        const word_t root = u_pathes[nu];

        assert(root == v_pathes[nv]);

        prevcycle_[i] = cuckoos_[root];
        int i2 = cuckoos_[root]; 
        cuckoos_[root] = i;
        if (i2 < 0) {
            continue;
        }
        const int rootdist = join_path(u_pathes, nu, v_pathes, nv);
        do {
            printf("chord found at cycleids %d %d\n", i2, i);

            const word_t u2 = cycle_edges_[i2].u;
            const word_t v2 = cycle_edges_[i2].v;
            unsigned int idx2_u = getpath(u2, us2);
            unsigned int idx2_v = getpath(v2, vs2);
            const word_t root2 = us2[idx2_u]; 

            assert(root2 == vs2[idx2_v] && root == root2);

            const int rootdist2 = join_path(us2, idx2_u, vs2, idx2_v);
            if (u_pathes[nu] == us2[idx2_u]) {
                const int len1 = sharedlen(u_pathes, nu, us2, idx2_u) + sharedlen(u_pathes, nu, vs2, idx2_v);
                const int len2 = sharedlen(v_pathes, nv, us2, idx2_u) + sharedlen(v_pathes, nv, vs2, idx2_v);
                if (len1 + len2 > 0) {
                    printf(" % 4d-cycle found At %d%%\n", cycle_lengths_[i] + cycle_lengths_[i2] - 2*(len1+len2), (int)(i*100L/n_cycles_));
                }
            } else {
                const int rd = rootdist - rootdist2;
                if (rd < 0) {
                    if (nu + rd > 0 && us2[idx2_u] == u_pathes[nu+rd]) {
                        const int len = sharedlen(u_pathes, nu+rd, us2, idx2_u) + sharedlen(u_pathes, nu+rd, vs2, idx2_v);
                        if (len) {
                            printf(" % 4d-cycle found At %d%%\n", cycle_lengths_[i] + cycle_lengths_[i2] - 2*len, (int)(i*100L/n_cycles_));
                        }
                    } else if (nv+rd > 0 && vs2[idx2_v] == v_pathes[nv+rd]) {
                        const int len = sharedlen(v_pathes, nv+rd, us2, idx2_u) + sharedlen(v_pathes, nv+rd, vs2, idx2_v);
                        if (len) {
                            printf(" % 4d-cycle found At %d%%\n", cycle_lengths_[i] + cycle_lengths_[i2] - 2*len, (int)(i*100L/n_cycles_));
                        }
                    }
                } else if (rd > 0) {
                    if (idx2_u-rd > 0 && u_pathes[nu] == us2[idx2_u-rd]) {
                        const int len = sharedlen(us2,idx2_u-rd,u_pathes,nu) + sharedlen(us2, idx2_u-rd, v_pathes, nv);
                        if (len) {
                            printf(" % 4d-cycle found At %d%%\n", cycle_lengths_[i] + cycle_lengths_[i2] - 2*len, (int)(i*100L/n_cycles_));
                        }
                    } else if (idx2_v-rd > 0 && v_pathes[nv] == vs2[idx2_v-rd]) {
                        const int len = sharedlen(vs2,idx2_v-rd,u_pathes,nu) + sharedlen(vs2, idx2_v-rd, v_pathes, nv);
                        if (len) {
                            printf(" % 4d-cycle found At %d%%\n", cycle_lengths_[i] + cycle_lengths_[i2] - 2*len, (int)(i*100L/n_cycles_));
                        }
                    }
                } // else cyles are disjoint
            }
            i2 = prevcycle_[i2];
        } while (i2 >= 0);
    }
}

// pathのcuckoo配列を取得する,同時にパス数を増加させる
//
// @param init_val 開始値
// @param pathes 玉突き結果を入れる配列
// @return cuckoo配列の最後のindexを返す
unsigned int Cyclebase::getpath(const u32 init_val, u32 *pathes) {
    unsigned int i = 0;
    pathes[i] = init_val;
    u32 j = init_val;
    while (path_counts_[j] > 0) {
        // パスの数が1以上である
        // パス数を増やす
        path_counts_[j] += 1;
        // 玉突き			
        j = cuckoos_[j];
        i += 1;
        if ((int)kMaxPathLen <= i) {
            // error
            path_error(init_val, pathes, i, j);
            assert(false);
        }
        pathes[i] = j;
    }
    return i;
}

void Cyclebase::path_error(const u32 init_val, u32* pathes, int i, u32 j) {
    while (true) {
        if (i == 0) {
            --i;
            break;
        } else {
            --i;
            if (pathes[i] == j) {
                break;
            }
        }
    }
    if (i < 0) {
        printf("maximum path length exceeded\n");
    }
    else { 
        printf("illegal % 4d-cycle from node %d\n", kMaxPathLen-i, init_val);
    }
    exit(0);		
}

// ???
int Cyclebase::join_path(
        const u32 *u_pathes,
        unsigned int &nu,
        const u32 *v_pathes,
        unsigned int &nv)
{
    int small = nu < nv ? nu : nv;
    nu -= small;
    nv -= small;
    for (; u_pathes[nu] != v_pathes[nv]; nu++, nv++) {
        small--;
    }
    return small;
}

void Cyclebase::print_record_edge(u32 i, u32 u, u32 v) {
    printf(" (%x,%x)", u, v);
}

void Cyclebase::print_solution(const u32 *u_pathes, int nu, const u32 *v_pathes, int nv) {
    printf("Nodes");

    u32 ni = 0;
    print_record_edge(ni, *u_pathes, *v_pathes);
    ni++;
    while (nu != 0) {
        nu--;
        print_record_edge(ni, u_pathes[(nu+1)&~1], u_pathes[nu|1]); // u's in even position; v's in odd
        ni++;
    }
    while (nv != 0) {
        nv--;
        print_record_edge(ni, v_pathes[nv|1], v_pathes[(nv+1)&~1]); // u's in odd position; v's in even
        ni++;
    } 

    printf("\n");
}

int Cyclebase::sharedlen(const u32 *u_pathes, int nu, const u32 *v_pathes, int nv) {
    int len = 0;
    for (; nu-- && nv-- && u_pathes[nu] == v_pathes[nv]; len++){
        ;
    }
    return len;
}
