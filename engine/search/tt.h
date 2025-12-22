//
// Created by Marvin Becker on 16.12.25.
//

/**
 * @file tt.h
 * @brief Transposition Table implementation for the chess engine search.
 *
 * This file defines the Transposition Table (TT) used by the alpha-beta / negamax
 * search to cache previously evaluated positions. Entries are indexed by a
 * 64-bit Zobrist hash and store score bounds and the best move found.
 *
 * The table is direct-mapped (one entry per index) and uses a simple replacement
 * policy based on generation and search depth.
 */

#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <core//move.h>

/**
 * @brief Global score constants used by the Transposition Table and search.
 *
 * These constants define the numeric ranges assumed by the engine:
 * - kMate:        Base value for mate scores. A "mate in N" is encoded as
 *                 ±(kMate - N).
 * - kMateWindow:  Window around ±kMate used to reliably detect mate scores.
 * - kInfinity:    A safe value larger than any possible evaluation or mate score,
 *                 suitable for initializing alpha/beta.
 *
 * @note These values must be consistent with the rest of the search and
 *       evaluation code.
 */
namespace tt_score_constants
{
inline constexpr int kMate = 32000;       // must be >> any evaluation score
inline constexpr int kMateWindow = 1000;  // window to detect mate scores
inline constexpr int kInfinity = 40000;   // must be > kMate
}

/**
 * @brief Type of score stored in a Transposition Table entry.
 *
 * The stored score can represent:
 * - EXACT:       The true score for this position at the stored depth.
 * - LOWER_BOUND: A lower bound on the true score (typically from a beta cutoff).
 * - UPPER_BOUND: An upper bound on the true score (typically from failing to
 *                raise alpha).
 *
 * These flags allow the TT probe to safely influence alpha/beta pruning without
 * changing the correctness of the search.
 */
enum class TTFlag : std::uint8_t
{
    EXACT = 0,
    LOWER_BOUND = 1, // score >= beta
    UPPER_BOUND = 2  // score <= alpha
};

/**
 * @brief Statistics collected for Transposition Table usage.
 *
 * These counters are primarily intended for debugging and performance analysis:
 * - probes:    Number of TT probe attempts.
 * - hits:      Number of successful probes (matching key found).
 * - stores:    Number of entries written to the table.
 * - replaces:  Number of times an existing entry was overwritten.
 */
struct TTStats {
    uint64_t probes_ = 0;
    uint64_t hits_ = 0;
    uint64_t stores_ = 0;
    uint64_t replaces_ = 0;
};


/**
 * @brief Fixed-size Transposition Table for caching search results.
 *
 * The Transposition Table stores information about previously visited positions
 * during the search, keyed by a 64-bit Zobrist hash. It is used to:
 * - Reuse previously computed scores (exact values or bounds).
 * - Trigger safe alpha/beta cutoffs.
 * - Improve move ordering by trying the stored best move first.
 *
 * Implementation notes:
 * - The table is direct-mapped (one entry per index).
 * - Collisions are resolved via a simple replacement policy based on generation
 *   and search depth.
 * - A generation counter is used to age entries across root searches.
 * - Mate scores are normalized with respect to ply so that "mate in N" remains
 *   consistent when retrieved at different depths.
 */
class TranspositionTable
{
public:
    /**
     * @brief Construct a TT with approximately ENTRIES slots.
     *
     * The actual size is rounded up to the next power of two.
     *
     * @param entries Requested number of entries (approximate).
     */
    explicit TranspositionTable(const std::size_t entries)
        : mask_(compute_mask(entries)), table_(mask_ + 1)
    {
    }

    /**
     * @brief Clear the entire table.
     *
     * This resets all entries. In practice you usually prefer using new_search() for aging,
     * and only clear() when you want a truly empty TT (e.g., new game).
     */
    void clear()
    {
        // Reset stats.
        stats_ = {};
        std::fill(table_.begin(), table_.end(), Entry{});
    }

    /**
     * @brief Start a new root search (iterative deepening iteration or fixed-depth search).
     *
     * Increments the generation counter so that older entries become cheaper to replace.
     */
    void new_search()
    {
        // Reset stats.
        stats_ = {};
        ++generation_;
        if (generation_ == 0) // overflow -> 0
            ++generation_;
    }

    /**
     * @brief Probe the TT for a position and optionally return a cutoff score.
     *
     * If a matching entry exists and was stored at least at the requested depth, this function
     * can return an exact score or a safe cutoff based on the stored bound flag.
     *
     * Additionally, out_best_move is set to the stored best move (or default Move{}) which
     * can be used for move ordering even if no cutoff is possible.
     *
     * The returned score, if any, is already de-normalized with respect to ply.
     *
     * @param key 64-bit Zobrist key of the position.
     * @param depth Remaining search depth requested.
     * @param alpha Current alpha bound.
     * @param beta Current beta bound.
     * @param ply Distance from the root (0 at root), used for mate score normalization.
     * @param out_score Set to the returned score if the probe causes a cutoff or exact hit.
     * @param out_best_move Set to the TT move for ordering if available.
     *
     * @return true if the probe returns a usable score (exact hit or cutoff), false otherwise.
     */
    bool probe(std::uint64_t key,
               int depth,
               int alpha,
               int beta,
               int ply,
               int& out_score,
               Move& out_best_move) const;

    /**
     * @brief Probe only for the stored best move.
     *
     * This is useful for move ordering in quiescence search or other contexts where
     * you don't want to apply bound logic.
     *
     * @param key 64-bit Zobrist key of the position.
     * @param out_best_move Set to the stored best move if present.
     *
     * @return true if a matching entry exists (and a move is available), false otherwise.
     */
    bool probe_move(std::uint64_t key, Move& out_best_move) const;

    /**
     * @brief Store a search result for a position.
     *
     * Mate scores are normalized before being written to the table.
     *
     * @param key 64-bit Zobrist key of the position.
     * @param depth Remaining depth at which SCORE was obtained.
     * @param score Score in the engine's internal units.
     * @param flag Exact / LowerBound / UpperBound describing SCORE.
     * @param ply Distance from root used to normalize mate scores.
     * @param best_move Best move found at this node (used for move ordering).
     */
    void store(std::uint64_t key,
               int depth,
               int score,
               TTFlag flag,
               int ply,
               const Move& best_move);

    const TTStats& get_stats() const { return stats_; }

private:
    /**
     * @brief Single entry stored in the Transposition Table.
     *
     * For a direct-mapped table, multiple different positions may map to the same
     * index. The replacement policy determines when an existing entry is overwritten.
     */
    struct Entry
    {
        std::uint64_t key_ = 0;
        int score_ = 0;
        int depth_ = -1;
        TTFlag flag_ = TTFlag::EXACT;
        std::uint8_t generation_ = 0;
        Move best_move_{};
    };

    /**
     * @brief Compute bitmask for indexing.
     *
     * We round the requested number of entries up to the next power of two (n),
     * and store a mask of (n - 1). Indexing is then: index = key & mask.
     */
    static std::size_t compute_mask(const std::size_t requested_entries)
    {
        // round to 2^n - 1 mask
        std::size_t n = 1;
        while (n < requested_entries) n <<= 1;
        return n - 1;
    }

    /**
     * @brief Normalize mate scores before storing them in the table.
     *
     * Mate scores depend on the distance from the root (ply). Normalization ensures
     * that a stored "mate in N" score can be retrieved correctly at a different ply.
     */
    static int to_tt_score(const int score, const int ply)
    {
        if (score > tt_score_constants::kMate - tt_score_constants::kMateWindow) return score + ply;
        if (score < -tt_score_constants::kMate + tt_score_constants::kMateWindow) return score - ply;
        return score;
    }

    /**
     * @brief De-normalize mate scores after loading them from the table.
     */
    static int from_tt_score(const int score, const int ply)
    {
        if (score > tt_score_constants::kMate - tt_score_constants::kMateWindow) return score - ply;
        if (score < -tt_score_constants::kMate + tt_score_constants::kMateWindow) return score + ply;
        return score;
    }

    /**
     * @brief Decide whether a new TT entry should replace the current one.
     *
     * Replacement policy:
     * - Always replace entries from an older generation.
     * - Otherwise, prefer entries searched to an equal or greater depth.
     */
    static bool is_replacement_better(const Entry& cur, const int new_depth, const std::uint8_t new_gen)
    {
        if (cur.generation_ != new_gen) return true;           // always replace old generation
        return new_depth >= cur.depth_;                        // same gen: prefer deeper
    }

    /**
     * @brief Compute table index from a Zobrist key.
     */
    [[nodiscard]] std::size_t index(const std::uint64_t KEY) const { return static_cast<std::size_t>(KEY) & mask_; }

    std::size_t mask_;
    std::vector<Entry> table_;
    std::uint8_t generation_ = 1;

    mutable TTStats stats_{};
};