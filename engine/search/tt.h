//
// Created by Marvin Becker on 16.12.25.
//

#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <core//move.h>

/**
 * @brief Global search score constants.
 *
 * These constants define the numeric ranges used by the engine:
 * - kMate:   Base value for mate scores (mate in N is encoded as ±(kMate - ply)).
 * - kMateWindow: Range around ±kMate that is treated as a mate score.
 * - kInfinity: Safe alpha/beta bound, larger than any evaluation or mate score.
 */
namespace tt_score_constants
{
inline constexpr int kMate = 32000;       // must be >> any evaluation score
inline constexpr int kMateWindow = 1000;  // window to detect mate scores
inline constexpr int kInfinity = 40000;   // must be > kMate
}

/**
 * @brief Type of score stored in the Transposition Table.
 *
 * The value stored for a position may be:
 * - Exact: the true score for the searched depth.
 * - LowerBound: score is a lower bound (often from a beta cutoff). I.e., true score >= stored score.
 * - UpperBound: score is an upper bound (often from failing to raise alpha). I.e., true score <= stored score.
 *
 * These flags allow the TT probe to safely cause alpha/beta cutoffs.
 */
enum class TTFlag : std::uint8_t
{
    Exact = 0,
    LowerBound = 1, // score >= beta
    UpperBound = 2  // score <= alpha
};


/**
 * @brief Fixed-size transposition table for caching search results.
 *
 * This table stores search information for previously visited positions keyed by a 64-bit Zobrist hash.
 * It can be used for:
 * - Reusing previously computed scores (exact or bounds) to prune the alpha-beta search.
 * - Improving move ordering by trying the best known TT move first.
 *
 * Implementation details:
 * - The table is 1-entry per index (direct-mapped). Collisions overwrite based on a simple replacement policy.
 * - The number of entries is rounded up to a power of two and indexed via bit masking.
 * - A generation counter is used to age entries between root searches.
 * - Mate scores are normalized w.r.t. ply (distance from root) so "mate in N" remains consistent.
 */
class TranspositionTable
{
public:
    /**
     * @brief Construct a TT with approximately ENTRIES slots.
     *
     * The actual size is rounded up to the next power of two.
     *
     * @param ENTRIES Requested number of entries (approximate).
     */
    explicit TranspositionTable(const std::size_t ENTRIES)
        : mask_(compute_mask(ENTRIES)), table_(mask_ + 1)
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
        std::fill(table_.begin(), table_.end(), Entry{});
    }

    /**
     * @brief Start a new root search (iterative deepening iteration or fixed-depth search).
     *
     * Increments the generation counter so that older entries become cheaper to replace.
     */
    void new_search()
    {
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
     * @param KEY 64-bit Zobrist key of the position.
     * @param DEPTH Remaining search depth requested.
     * @param ALPHA Current alpha bound.
     * @param BETA Current beta bound.
     * @param PLY Distance from the root (0 at root), used for mate score normalization.
     * @param out_score Set to the returned score if the probe causes a cutoff or exact hit.
     * @param out_best_move Set to the TT move for ordering if available.
     *
     * @return true if the probe returns a usable score (exact hit or cutoff), false otherwise.
     */
    bool probe(std::uint64_t KEY,
               int DEPTH,
               int ALPHA,
               int BETA,
               int PLY,
               int& out_score,
               Move& out_best_move) const;

    /**
     * @brief Probe only for the stored best move.
     *
     * This is useful for move ordering in quiescence search or other contexts where
     * you don't want to apply bound logic.
     *
     * @param KEY 64-bit Zobrist key of the position.
     * @param out_best_move Set to the stored best move if present.
     *
     * @return true if a matching entry exists (and a move is available), false otherwise.
     */
    bool probe_move(std::uint64_t KEY, Move& out_best_move) const;

    /**
     * @brief Store a search result for a position.
     *
     * @param KEY 64-bit Zobrist key of the position.
     * @param DEPTH Remaining depth at which SCORE was obtained.
     * @param SCORE Score in the engine's internal units.
     * @param FLAG Exact / LowerBound / UpperBound describing SCORE.
     * @param PLY Distance from root used to normalize mate scores.
     * @param BEST_MOVE Best move found at this node (used for move ordering).
     */
    void store(std::uint64_t KEY,
               int DEPTH,
               int SCORE,
               TTFlag FLAG,
               int PLY,
               const Move& BEST_MOVE);

private:
    /**
     * @brief Single TT entry stored at an index.
     *
     * For direct-mapped tables, multiple different keys can map to the same index.
     * The replacement policy decides when to overwrite an existing entry.
     */
    struct Entry
    {
        std::uint64_t key = 0;
        int score = 0;
        int depth = -1;
        TTFlag flag = TTFlag::Exact;
        std::uint8_t generation = 0;
        Move best_move{};
    };

    /**
     * @brief Compute bitmask for indexing.
     *
     * We round the requested number of entries up to the next power of two (n),
     * and store a mask of (n - 1). Indexing is then: index = key & mask.
     */
    static std::size_t compute_mask(const std::size_t REQUESTED_ENTRIES)
    {
        // round to 2^n - 1 mask
        std::size_t n = 1;
        while (n < REQUESTED_ENTRIES) n <<= 1;
        return n - 1;
    }

    /**
     * @brief Normalize mate scores before storing.
     *
     * Without normalization, a mate score depends on the current ply (distance from root),
     * which would make retrieving it at a different ply incorrect ("mate in N" changes).
     */
    static int to_tt_score(const int SCORE, const int PLY)
    {
        if (SCORE > tt_score_constants::kMate - tt_score_constants::kMateWindow) return SCORE + PLY;
        if (SCORE < -tt_score_constants::kMate + tt_score_constants::kMateWindow) return SCORE - PLY;
        return SCORE;
    }

    /**
     * @brief Undo mate score normalization after loading.
     */
    static int from_tt_score(const int SCORE, const int PLY)
    {
        if (SCORE > tt_score_constants::kMate - tt_score_constants::kMateWindow) return SCORE - PLY;
        if (SCORE < -tt_score_constants::kMate + tt_score_constants::kMateWindow) return SCORE + PLY;
        return SCORE;
    }

    /**
     * @brief Decide whether a new entry should replace the current one.
     *
     * Current policy:
     * - Always replace entries from older generations.
     * - Otherwise prefer deeper (or equal depth) entries.
     */
    static bool is_replacement_better(const Entry& cur, const int NEW_DEPTH, const std::uint8_t NEW_GEN)
    {
        if (cur.generation != NEW_GEN) return true;           // always replace old generation
        return NEW_DEPTH >= cur.depth;                        // same gen: prefer deeper
    }

    /**
     * @brief Compute table index from a Zobrist key.
     */
    [[nodiscard]] std::size_t index(const std::uint64_t KEY) const { return static_cast<std::size_t>(KEY) & mask_; }

    std::size_t mask_;
    std::vector<Entry> table_;
    std::uint8_t generation_ = 1;
};