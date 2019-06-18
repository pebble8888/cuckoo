#include "cyclebase.h"

Cyclebase::Cyclebase(uint32_t num_nodes)
    : num_nodes_(num_nodes)
{
    nodes_ = new word_t[num_nodes_];
    path_counts_ = new uint32_t[num_nodes_];
    for (int i = 0; i < num_nodes_; ++i) {
        nodes_[i] = 0;
        path_counts_ = 0;
    }
}

Cyclebase::~Cyclebase()
{
    delete [] nodes_;
    delete [] path_counts_;
}

// 何をリセットする? 
void Cyclebase::reset_count(void)
{
    for (auto i = 0; i < num_nodes_; ++i) {
        nodes_[i] = kNone; // for prevcycle nil
        path_counts_[i] = 0;
    }
    num_cycles_ = 0;
}

/**
 * @brief 
 * @param u_bare 偶奇なし [0, NEDGES)
 * @param v_bare 偶奇なし [0, NEDGES)
 */
void Cyclebase::add_edge(const uint32_t u_bare, const uint32_t v_bare)
{
    uint32_t u_pathes[kMaxPathLen]; // work領域
    uint32_t v_pathes[kMaxPathLen]; // work領域
    const uint32_t u = 2 * u_bare; // u is even
    const uint32_t v = 2 * v_bare + 1; // v is odd
    unsigned int u_path_index = append_path(u, u_pathes);
    unsigned int v_path_index = append_path(v, v_pathes);
    if (u_pathes[u_path_index] == v_pathes[v_path_index]) {
        // パス配列の最後の値(つまり先頭の値)が一致している: サイクルが見つかった
        // 要求される長さとは限らない
        
        // このとき path_counts_ は増加させない(なぜか?)

        join_path(u_pathes, u_path_index, v_pathes, v_path_index);
        const int len = u_path_index + v_path_index + 1;

        printf("% 4d-cycle found in add_edge()\n", len);

        // サイクルの位置を保存する
        edges_[num_cycles_].u = u;
        edges_[num_cycles_].v = v;
        // サイクルの長さを保存する
        cycle_lengths_[num_cycles_] = len;
        // サイクル数を増加
        num_cycles_ += 1;
        if (len == PROOFSIZE) {
            // 要求サイズを満たしたサイクルである
            print_solution(u_pathes, u_path_index, v_pathes, v_path_index);
        }
        assert(num_cycles_ < MAXCYCLES);
    } else if (u_path_index < v_path_index) {
        // u側サイクルの方が少ない
        // 少ない側(u)のパスを伸ばす
        const auto i = u_pathes[u_path_index];
        ++path_counts_[i];
        // 少ない側(u)のノードを更新する
        for (int k = u_path_index; k > 0; --k) {
            const auto j = u_pathes[k];
            nodes_[j] = u_pathes[k-1];
        }
        nodes_[u] = v;
    } else {
        // v側サイクルの方が少ないもしくはサイクルなし
        // 少ない側(v)のパスを伸ばす
        const auto i = v_pathes[v_path_index];
        ++path_counts_[i];
        // 少ない側(v)のノードを更新する
        for (int k = v_path_index; k > 0; --k) {
            const auto j = v_pathes[k];
            nodes_[j] = v_pathes[k-1];
        }
        nodes_[v] = u;
    }
}

// TODO:メソッド名がわかりづらい
void Cyclebase::cycles(void)
{
    uint32_t u_pathes[kMaxPathLen];
    uint32_t v_pathes[kMaxPathLen];
    word_t u_pathes_2[kMaxPathLen];
    word_t v_pathes_2[kMaxPathLen];

    uint32_t prevcycle[MAXCYCLES];

    for (int i = 0; i < num_cycles_; i++) {
        const word_t u3 = edges_[i].u;
        const word_t v3 = edges_[i].v;
        unsigned int u_path_index = append_path(u3, u_pathes);
        unsigned int v_path_index = append_path(v3, v_pathes);
        const word_t root = u_pathes[u_path_index];

        assert(root == v_pathes[v_path_index]);

        prevcycle[i] = nodes_[root];
        int i2 = nodes_[root]; 
        nodes_[root] = i;
        if (i2 < 0) {
            continue;
        }
        const int rootdist = join_path(u_pathes, u_path_index, v_pathes, v_path_index);
        do {
            printf("chord found at cycleids %d %d\n", i2, i);

            const word_t u2 = edges_[i2].u;
            const word_t v2 = edges_[i2].v;
            unsigned int u_idx2 = append_path(u2, u_pathes_2);
            unsigned int v_idx2 = append_path(v2, v_pathes_2);
            const word_t root2 = u_pathes_2[u_idx2]; 

            assert(root2 == v_pathes_2[v_idx2] && root == root2);

            const int rootdist2 = join_path(u_pathes_2, u_idx2, v_pathes_2, v_idx2);
            if (u_pathes[u_path_index] == u_pathes_2[u_idx2]) {
                const int len1 = sharedlen(u_pathes, u_path_index, u_pathes_2, u_idx2) + sharedlen(u_pathes, u_path_index, v_pathes_2, v_idx2);
                const int len2 = sharedlen(v_pathes, v_path_index, u_pathes_2, u_idx2) + sharedlen(v_pathes, v_path_index, v_pathes_2, v_idx2);
                if (len1 + len2 > 0) {
                    printf(" % 4d-cycle found At %d%%\n", cycle_lengths_[i] + cycle_lengths_[i2] - 2*(len1+len2), (int)(i*100L/num_cycles_));
                }
            } else {
                const int rd = rootdist - rootdist2;
                if (rd < 0) {
                    if (u_path_index + rd > 0 && u_pathes_2[u_idx2] == u_pathes[u_path_index + rd]) {
                        const int len = sharedlen(u_pathes, u_path_index + rd, u_pathes_2, u_idx2) + sharedlen(u_pathes, u_path_index + rd, v_pathes_2, v_idx2);
                        if (len > 0) {
                            printf(" % 4d-cycle found At %d%%\n", cycle_lengths_[i] + cycle_lengths_[i2] - 2*len, (int)(i*100L/num_cycles_));
                        }
                    } else if (v_path_index + rd > 0 && v_pathes_2[v_idx2] == v_pathes[v_path_index + rd]) {
                        const int len = sharedlen(v_pathes, v_path_index + rd, u_pathes_2, u_idx2) + sharedlen(v_pathes, v_path_index + rd, v_pathes_2, v_idx2);
                        if (len > 0) {
                            printf(" % 4d-cycle found At %d%%\n", cycle_lengths_[i] + cycle_lengths_[i2] - 2*len, (int)(i*100L/num_cycles_));
                        }
                    }
                } else if (rd > 0) {
                    if (u_idx2-rd > 0 && u_pathes[u_path_index] == u_pathes_2[u_idx2 - rd]) {
                        const int len = sharedlen(u_pathes_2, u_idx2 - rd, u_pathes, u_path_index) + sharedlen(u_pathes_2, u_idx2 - rd, v_pathes, v_path_index);
                        if (len > 0) {
                            printf(" % 4d-cycle found At %d%%\n", cycle_lengths_[i] + cycle_lengths_[i2] - 2*len, (int)(i*100L/num_cycles_));
                        }
                    } else if (v_idx2-rd > 0 && v_pathes[v_path_index] == v_pathes_2[v_idx2-rd]) {
                        const int len = sharedlen(v_pathes_2, v_idx2 - rd, u_pathes, u_path_index) + sharedlen(v_pathes_2, v_idx2 - rd, v_pathes, v_path_index);
                        if (len > 0) {
                            printf(" % 4d-cycle found At %d%%\n", cycle_lengths_[i] + cycle_lengths_[i2] - 2*len, (int)(i*100L/num_cycles_));
                        }
                    }
                } // else cyles are disjoint
            }
            i2 = prevcycle[i2];
        } while (i2 >= 0);
    }
}

/** 
 * @brief   pathのcuckoo配列を取得する,同時にパス数を増加させる
 * @param [in]  newval : 今回追加する値 (odd or even) [0, NEDGES)
 * @param [out] pathes : 玉突き結果を入れる配列の先頭アドレス, 先頭側に最後に追加したものが入る 配列の最大長はkMaxPathLen
 * @return  最後に追加したパスのインデックス(1以上の場合,パスの数-1)を返す
 * @note    path_count_の値のみを変更しnodes_の値は変更しない  
 */
unsigned int Cyclebase::append_path(const uint32_t newval, uint32_t * pathes)
{
    unsigned int path_index = 0;
    pathes[path_index] = newval;
    uint32_t j = newval;
    while (path_counts_[j] > 0) {
        // パスの数が1以上である,nodes_ にはすでに値が入っている

        // パス数を増やす
        ++path_counts_[j];
        // 入っている値を取り出す
        j = nodes_[j];
        ++path_index;
        // パスが最大値を超えていないかチェックする
        if (path_index >= (int)kMaxPathLen) {
            // fatal error
            path_error(newval, pathes, path_index, j);
            assert(false);
        }
        // パスの値をチェックする
        pathes[path_index] = j;
    }
    return path_index;
}

void Cyclebase::path_error(const uint32_t init_val, uint32_t* pathes, int i, uint32_t j)
{
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

/**
 * @brief 
 * @param [in]      u_pathes
 * @param [in/out]  u_path_index
 * @param [in]      v_pathes
 * @param [in/out]  v_path_index
 * @return 
 */
int Cyclebase::join_path(
        const uint32_t *u_pathes,
        unsigned int &u_path_index,
        const uint32_t *v_pathes,
        unsigned int &v_path_index)
{
    // u_path_index = 2, v_path_index = 3 の場合
    // small = 2
    int small = std::min<int>(u_path_index, v_path_index);
    u_path_index -= small;
    v_path_index -= small;

    // u_path_index = 0
    // v_path_index = 1

    while (u_pathes[u_path_index] != v_pathes[v_path_index]) {
        small--;
        ++u_path_index;
        ++v_path_index;
    }
    return small;
}

void Cyclebase::print_record_edge(uint32_t u, uint32_t v)
{
    printf(" (%x,%x)", u, v);
}

void Cyclebase::print_solution(
    const uint32_t *u_pathes,
    int u_path_index,
    const uint32_t *v_pathes,
    int v_path_index)
{
    printf("print_solution() Nodes");

    uint32_t ni = 0;
    print_record_edge(*u_pathes, *v_pathes);
    ni++;
    while (u_path_index != 0) {
        u_path_index--;
        // u's in even position; v's in odd
        print_record_edge(u_pathes[(u_path_index+1)&~1], u_pathes[u_path_index|1]);
        ni++;
    }
    while (v_path_index != 0) {
        v_path_index--;
        // u's in odd position; v's in even
        print_record_edge(v_pathes[v_path_index|1], v_pathes[(v_path_index+1)&~1]);
        ni++;
    } 

    printf("\n");
}

/**
 * u_pathes = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
 * v_pathes = [0, 1, 2, 3, 4, 0, 6, 7, 8, 9]
 * u_path_index = 9
 * v_path_index = 9
 * の場合は len = 3 となる
 */
int Cyclebase::sharedlen(const uint32_t *u_pathes,
                         int u_path_index,
                         const uint32_t *v_pathes,
                         int v_path_index)
{
    int len = 0;
    while (true) {
        if (u_path_index > 0 && v_path_index > 0) {
            --u_path_index;
            --v_path_index;
            if (u_pathes[u_path_index] == v_pathes[v_path_index]) {
                ++len;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    return len;
}

