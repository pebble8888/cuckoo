#include <utility>
#include <stdio.h>
#include <assert.h>
#include <set>

#include "cuckoo.h"

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
	Cyclebase() = delete;
	Cyclebase(uint32_t n_cuckoo)
	    : n_cuckoo_(n_cuckoo)
	{
        alloc();
	}
    ~Cyclebase()
    {
        freemem();
    }

    // cuckoos_配列の先頭アドレス
    const word_t * cuckoo_address(void) const { return cuckoos_; }

    void reset_count(void);

    // エッジの追加
    // u0: 左側のハッシュインデクス
    // v0: 右側のハッシュインデクス
    void add_edge(const u32 u0, const u32 v0);
    
    // エッジを追加するごとに呼び出す
    void cycles(void);

private:
    void alloc(void);
    void freemem(void);

    unsigned int getpath(const u32 init_val, u32 * pathes);

	void path_error(const u32 init_val, u32* pathes, int i, u32 j);

    static int join_path(
            const u32 *u_pathes,
            unsigned int &nu,
            const u32 *v_pathes,
            unsigned int &nv);

    static void print_record_edge(u32 i, u32 u, u32 v);
    static void print_solution(const u32 *u_pathes, int nu, const u32 *v_pathes, int nv);
    static int sharedlen(const u32 *u_pathes, int nu, const u32 *v_pathes, int nv);

    // should avoid different values of kMaxPathLen in different threads of one process
    static constexpr u32 kMaxPathLen = 16 << (EDGEBITS/3);
    // サイクルの数,サイクルとは一つの点から始まり,その点に戻るグラフの繋がりを指す.
    int         n_cycles_ = 0;

    // ? 初期値は-1(unsigned なので最大値になる) 要素数: n_cuckoo_
    word_t *    cuckoos_ = nullptr;

    // n番目のスロットのパスの数 スロットのインデックスの意味は? 要素数: n_cuckoo_
    u32 *       path_counts_ = nullptr;

    Edge        cycle_edges_[MAXCYCLES]; // ?
    u32         cycle_lengths_[MAXCYCLES]; // ?
    u32         prevcycle_[MAXCYCLES]; // ?
	const uint32_t n_cuckoo_; // コンストラクタで初期化
};

