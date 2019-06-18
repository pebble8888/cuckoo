#pragma once

#include <utility>
#include <stdio.h>
#include <assert.h>
#include <set>
#include <vector>

#include "cuckoo.h"

#ifndef MAXCYCLES
#define MAXCYCLES 64 // single byte
#endif

struct Edge {
    uint32_t u;
    uint32_t v;
    Edge()
        : u(0)
        , v(0)
    {}

    Edge(uint32_t x, uint32_t y)
        : u(x)
        , v(y)
    {}
};

class Cyclebase {
public:
	Cyclebase() = delete;
    Cyclebase(uint32_t num_nodes);
    ~Cyclebase();

    void reset_count(void);

    // エッジの追加
    // bare_u: 左側のハッシュインデックス
    // bare_v: 右側のハッシュインデックス
    void add_edge(const uint32_t bare_u, const uint32_t bare_v);
    
    // エッジを追加するごとに呼び出す
    void cycles(void);

private:
    // should avoid different values of kMaxPathLen in different threads of one process
    static constexpr uint32_t kMaxPathLen = 16 << (EDGEBITS/3);

    static constexpr word_t kNone = -1;

    unsigned int append_path(const uint32_t newval, uint32_t * pathes);
	void path_error(const uint32_t init_val, uint32_t* pathes, int i, uint32_t j);
    static int join_path(
            const uint32_t *u_pathes,
            unsigned int &u_idx,
            const uint32_t *v_pathes,
            unsigned int &v_idx);
    static void print_record_edge(uint32_t u, uint32_t v);
    static void print_solution(const uint32_t *u_pathes, int u_idx, const uint32_t *v_pathes, int v_idx);
    static int sharedlen(const uint32_t *u_pathes, int u_idx, const uint32_t *v_pathes, int v_idx);

    /// サイクルの数,サイクルとは一つの点から始まり,その点に戻るグラフの繋がりを指す.
    int num_cycles_ = 0;
    
    /// コンストラクタで初期化する
	const uint32_t num_nodes_;
    /**
     * @brief ? 初期値は-1(unsigned なので最大値になる)
     *        要素数: num_nodes_
     */
    word_t* nodes_ = nullptr;
    /// n番目のスロットのパスの数 スロットのインデックスの意味は? 要素数: num_nodes_
    uint32_t* path_counts_ = nullptr;
    /// エッジ
    Edge edges_[MAXCYCLES];
    /// サイクルの長さ
    uint32_t cycle_lengths_[MAXCYCLES];
};

