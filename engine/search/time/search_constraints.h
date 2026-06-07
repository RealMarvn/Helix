//
// Created by Marvin Becker on 18.12.25.
//

#pragma once

#include <cstdint>
#include "time_control.h"

/**
 * @brief Defines how the search is controlled and when it should terminate.
 *
 * The search type determines whether the engine searches
 * - with tournament-style time management,
 * - for a fixed amount of time,
 * - to a fixed depth,
 * - with a node limit,
 * - infinitely until externally stopped,
 * - or in ponder mode (thinking during opponent time).
 */
enum class SearchType : uint8_t
{
    Normal,      ///< Tournament mode: time-managed iterative deepening (default UCI behavior)
    FixedTime,   ///< Search for a fixed amount of time (movetime)
    FixedDepth,  ///< Search to a fixed depth only (no time control)
    NodeLimit,   ///< Search until a fixed number of nodes is reached
    Infinite,    ///< Search indefinitely until externally stopped (UCI "go infinite")
    Ponder       ///< Ponder mode: think during opponent's time
};

/**
 * @brief Immutable constraints controlling a single search invocation.
 *
 * SearchConstraints bundle all stopping conditions for one search,
 * including depth limits, node limits, and time control.
 *
 * The structure is treated as read-only during the search and is
 * initialized once before the search starts.
 */
struct SearchConstraints
{

    /**
     * @brief Selected search control mode.
     *
     * Determines which stopping criteria are active during the search.
     */
    SearchType mode_ = SearchType::Normal;

    /**
     * @brief Fixed time limit for the search in milliseconds.
     *
     * Only used when @ref SearchType::FixedTime is active.
     * A negative value indicates that no fixed move time is set.
     */
    int movetime_ms_ = -1;

    /**
     * @brief Fixed search depth.
     *
     * Only used when @ref SearchType::FixedDepth is active.
     * A negative value indicates that no fixed depth is set.
     */
    int depth_ = -1;

    /**
     * @brief Maximum number of nodes to be searched.
     *
     * Only used when @ref SearchType::NodeLimit is active.
     * A negative value indicates that no node limit is set.
     */
    long long nodes_ = -1;

    /**
     * @brief Time control budget for the current search.
     *
     * Contains soft and hard deadlines used for time-managed searches.
     * For non-time-based searches (e.g. fixed depth), this structure
     * is left unarmed (all deadlines set to zero).
     */
    TimeControl budget_;

    /** @name Convenience helpers
     *  @{ */
    [[nodiscard]] bool has_fixed_depth() const { return depth_ >= 0; }
    [[nodiscard]] bool has_node_limit() const { return nodes_ >= 0; }
    [[nodiscard]] bool has_movetime() const { return movetime_ms_ >= 0; }
    /** @} */
};
