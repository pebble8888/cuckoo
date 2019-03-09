#include <utility>
#include <stdio.h>
#include <assert.h>
#include <set>

#ifndef MAXCYCLES
#define MAXCYCLES 64 // single byte
#endif

struct Edge {
    u32 u;
    u32 v;
    Edge() : u(0), v(0)
    {
    }
    Edge(u32 x, u32 y) : u(x), v(y)
    {
    }
};

class Cyclebase {
public:
    const word_t * cuckoo_address(void) const {
        return cuckoo_;
    }
    void alloc() {
        cuckoo_ = new word_t[NCUCKOO];
        memset(cuckoo_, 0, sizeof(word_t) * NCUCKOO);
        path_count_ = new u32[NCUCKOO];
        memset(path_count_, 0, sizeof(u32) * NCUCKOO);
    }

    void freemem() { // not a destructor, as memory may have been allocated elsewhere, bypassing alloc()
        delete [] cuckoo_;
        delete [] path_count_;
    }

    void reset_count() {
        for (auto i = 0; i < NCUCKOO; ++i) {
            cuckoo_[i] = -1; // for prevcycle nil
            path_count_[i] = 0;
        }
        ncycles_ = 0;
    }

    // エッジの追加
    // u0: 左側のハッシュインデクス
    // v0: 右側のハッシュインデクス
    void add_edge(const u32 u0, const u32 v0) {
        u32 us[MAXPATHLEN]; // work領域
        u32 vs[MAXPATHLEN]; // work領域
        const u32 u = u0 << 1; // u0の値を2倍する
        const u32 v = (v0 << 1) | 1; // v0の値を2倍して1を足す
        unsigned int nu = getpath(u, us);
        unsigned int nv = getpath(v, vs);
        if (us[nu] == vs[nv]) {
            // cuckoo配列の最後の値が一致している: found cycle
            // 要求される長さとは限らない
            
            // このときpath_count_は増加させない(なぜか?)

            join_path(us, nu, vs, nv);
            const int len = nu + nv + 1;

            printf("% 4d-cycle found\n", len);

            // サイクルの位置を保存する
            cycle_edges_[ncycles_].u = u;
            cycle_edges_[ncycles_].v = v;
            // サイクルの長さを保存する
            cycle_lengths_[ncycles_] = len;
            // サイクル数を増加
            ncycles_ += 1;
            if (len == PROOFSIZE) {
                // 要求サイズを満たしたサイクルである
                print_solution(us, nu, vs, nv);
            }
            assert(ncycles_ < MAXCYCLES);
        } else if (nu < nv) {
            // u側サイクルの方が少ない
			// 少ない側(u)のパスを伸ばす
			const auto i = us[nu];
            path_count_[i] += 1;
			// 少ない側(u)のカッコーを更新する
			for (int k = nu; k > 0; --k) {
				const auto j = us[k];
				if (cuckoo_[j] != -1) {
					printf("cuckoo!\n");
				}
                cuckoo_[j] = us[k-1];
				printf("cuckoo_[%d]: %d\n", j, cuckoo_[j]);				
            }
			if (cuckoo_[u] != -1) {
				printf("cuckoo!\n");
			}
            cuckoo_[u] = v;
			printf("cuckoo_[%d]: %d\n", u, cuckoo_[u]);							
        } else {
			// v側サイクルの方が少ないもしくはサイクルなし
			// 少ない側(v)のパスを伸ばす
			const auto i = vs[nv];
            path_count_[i] += 1;
			// 少ない側(v)のカッコーを更新する
			for (int k = nv; k > 0; --k) {
				const auto j = vs[k];
				if (cuckoo_[j] != -1) {
					printf("cukcoo!\n");
				}				
				cuckoo_[j] = vs[k-1];
				printf("cuckoo_[%d]: %d\n", j, cuckoo_[j]);				
            }
			if (cuckoo_[v] != -1) {
				printf("cuckoo!\n");
			}
            cuckoo_[v] = u;
			printf("cuckoo_[%d]: %d\n", v, cuckoo_[v]);							
        }
    }

    // ??
    void cycles(void) {
        u32 us[MAXPATHLEN];
        u32 vs[MAXPATHLEN];
        word_t us2[MAXPATHLEN];
        word_t vs2[MAXPATHLEN];
        for (int i = 0; i < ncycles_; i++) {
            const word_t u3 = cycle_edges_[i].u;
            const word_t v3 = cycle_edges_[i].v;
            unsigned int nu = getpath(u3, us);
            unsigned int nv = getpath(v3, vs);
            const word_t root = us[nu];

            assert(root == vs[nv]);

            prevcycle_[i] = cuckoo_[root];
            int i2 = cuckoo_[root]; 
            cuckoo_[root] = i;
            if (i2 < 0) {
                continue;
            }
            const int rootdist = join_path(us, nu, vs, nv);
            do {
                printf("chord found at cycleids %d %d\n", i2, i);

                const word_t u2 = cycle_edges_[i2].u;
                const word_t v2 = cycle_edges_[i2].v;
                unsigned int idx2_u = getpath(u2, us2);
                unsigned int idx2_v = getpath(v2, vs2);
                const word_t root2 = us2[idx2_u]; 

                assert(root2 == vs2[idx2_v] && root == root2);

                const int rootdist2 = join_path(us2, idx2_u, vs2, idx2_v);
                if (us[nu] == us2[idx2_u]) {
                    const int len1 = sharedlen(us, nu, us2, idx2_u) + sharedlen(us, nu, vs2, idx2_v);
                    const int len2 = sharedlen(vs, nv, us2, idx2_u) + sharedlen(vs, nv, vs2, idx2_v);
                    if (len1 + len2 > 0) {
                        printf(" % 4d-cycle found At %d%%\n", cycle_lengths_[i] + cycle_lengths_[i2] - 2*(len1+len2), (int)(i*100L/ncycles_));
                    }
                } else {
                    const int rd = rootdist - rootdist2;
                    if (rd < 0) {
                        if (nu + rd > 0 && us2[idx2_u] == us[nu+rd]) {
                            const int len = sharedlen(us, nu+rd, us2, idx2_u) + sharedlen(us, nu+rd, vs2, idx2_v);
                            if (len) {
                                printf(" % 4d-cycle found At %d%%\n", cycle_lengths_[i] + cycle_lengths_[i2] - 2*len, (int)(i*100L/ncycles_));
                            }
                        } else if (nv+rd > 0 && vs2[idx2_v] == vs[nv+rd]) {
                            const int len = sharedlen(vs, nv+rd, us2, idx2_u) + sharedlen(vs, nv+rd, vs2, idx2_v);
                            if (len) {
                                printf(" % 4d-cycle found At %d%%\n", cycle_lengths_[i] + cycle_lengths_[i2] - 2*len, (int)(i*100L/ncycles_));
                            }
                        }
                    } else if (rd > 0) {
                        if (idx2_u-rd > 0 && us[nu] == us2[idx2_u-rd]) {
                            const int len = sharedlen(us2,idx2_u-rd,us,nu) + sharedlen(us2, idx2_u-rd, vs, nv);
                            if (len) {
                                printf(" % 4d-cycle found At %d%%\n", cycle_lengths_[i] + cycle_lengths_[i2] - 2*len, (int)(i*100L/ncycles_));
                            }
                        } else if (idx2_v-rd > 0 && vs[nv] == vs2[idx2_v-rd]) {
                            const int len = sharedlen(vs2,idx2_v-rd,us,nu) + sharedlen(vs2, idx2_v-rd, vs, nv);
                            if (len) {
                                printf(" % 4d-cycle found At %d%%\n", cycle_lengths_[i] + cycle_lengths_[i2] - 2*len, (int)(i*100L/ncycles_));
                            }
                        }
                    } // else cyles are disjoint
                }
                i2 = prevcycle_[i2];
            } while (i2 >= 0);
        }
    }

private:
    // @brief pathのcuckoo配列を取得する,同時にパス数を増加させる
    // @param initial 開始値
    // @param s 玉突き結果を入れる配列
    // @return cuckoo配列の最後のindexを返す
    unsigned int getpath(const u32 initial, u32 *s)
    {
        unsigned int i = 0;
        s[i] = initial;
        u32 j = initial;
        while (path_count_[j] > 0) {
            // パスの数が1以上である
			// パス数を増やす
            path_count_[j] += 1;
			// 玉突き			
            j = cuckoo_[j];
			i += 1;
            if ((int)MAXPATHLEN <= i) {
                // error
				path_error(initial, s, i, j);
				assert(false);
            }
            s[i] = j;
        }
        return i;
    }
	
	void path_error(const u32 initial, u32* s, int i, u32 j) {
		while (true) {
			if (i == 0) {
				--i;
				break;
			} else {
				--i;
				if (s[i] == j) {
					break;
				}
			}
		}
		if (i < 0) {
			printf("maximum path length exceeded\n");
		}
		else { 
			printf("illegal % 4d-cycle from node %d\n", MAXPATHLEN-i, initial);
		}
		exit(0);		
	}

    static int join_path(const u32 *us,
                 unsigned int &nu,
                 const u32 *vs,
                 unsigned int &nv)
    {

        int small = nu < nv ? nu : nv;
        nu -= small;
        nv -= small;
        for (; us[nu] != vs[nv]; nu++, nv++) {
            small--;
        }
        return small;
    }

    static void print_record_edge(u32 i, u32 u, u32 v) {
        printf(" (%x,%x)", u, v);
    }

    static void print_solution(const u32 *us, int nu, const u32 *vs, int nv) {
        printf("Nodes");

        u32 ni = 0;
        print_record_edge(ni, *us, *vs);
        ni++;
        while (nu != 0) {
            nu--;
            print_record_edge(ni++, us[(nu+1)&~1], us[nu|1]); // u's in even position; v's in odd
        }
        while (nv != 0) {
            nv--;
            print_record_edge(ni++, vs[nv|1], vs[(nv+1)&~1]); // u's in odd position; v's in even
        } 

        printf("\n");
    }

    static int sharedlen(const u32 *us, int nu, const u32 *vs, int nv) {
        int len = 0;
        for (; nu-- && nv-- && us[nu] == vs[nv]; len++){
            ;
        }
        return len;
    }

    // should avoid different values of MAXPATHLEN in different threads of one process
    static constexpr u32 MAXPATHLEN = 16 << (EDGEBITS/3);
    int ncycles_; // ?
    word_t * cuckoo_ = nullptr; // ? 初期値は-1(unsigned なので最大値になる)
    u32 * path_count_ = nullptr; // n番目のスロットのパスの数
    Edge cycle_edges_[MAXCYCLES]; // ?
    u32 cycle_lengths_[MAXCYCLES]; // ?
    u32 prevcycle_[MAXCYCLES]; // ?
};

