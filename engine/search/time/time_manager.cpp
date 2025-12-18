//
// Created by Marvin Becker on 16.12.25.
//

#include "../time_manager.h"
#include <chrono>

namespace search::time
{

int TimeManager::default_reserve_ms(const int my_time_ms)
{
    // ~2% reserve, min 100ms.
    return std::max(100, my_time_ms / 50);
}

int TimeManager::default_moves_left(const int movestogo)
{
    return (movestogo > 0) ? movestogo : 30;
}

TimeControl TimeManager::compute_budget(const int side_to_move, const UciTimeControl& tc)
{
    // White = 0 | Black = 1.
    const int my_time = (side_to_move == 0) ? tc.wtime : tc.btime;
    const int my_inc = (side_to_move == 0) ? tc.winc : tc.binc;

    // Fallback if no clock infos.
    if (my_time <= 0)
    {
        return TimeControl{
            1400, // soft
            1900, // hard
            2000  // total
        };
    }

    const int reserve = (tc.reserve_ms > 0) ? tc.reserve_ms : default_reserve_ms(my_time);

    // Time, we can only use to calculate.
    int safe_time = my_time - tc.overhead_ms - reserve;
    if (safe_time < 0)
        safe_time = 0;

    const int moves_left = default_moves_left(tc.movestogo);

    // Initial budget.
    int budget = safe_time / std::max(1, moves_left);

    // Increment.
    if (my_inc > 0)
        budget += (my_inc * 8) / 10; // 80%

    // Never more than 25% of the rest-time
    const int hard_cap = std::max(10, safe_time / 4);
    budget = std::clamp(budget, 10, hard_cap);

    const int soft_ms = std::max(1, (budget * 70) / 100);
    const int hard_ms = std::max(1, (budget * 95) / 100);

    return TimeControl{soft_ms, hard_ms, budget};
}

std::int64_t TimeManager::now_ms()
{
    using clock = std::chrono::steady_clock;
    const auto now = clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

void TimeManager::init_search(SearchConstraints& search_limits)
{
    if (search_limits.mode == SearchType::FixedTime)
    {
        const int total = search_limits.movetime_ms;

        // 70% for soft.
        const int soft = std::max(1, (total * 70) / 100);

        // 95% for hard.
        const int hard = std::max(1, (total * 95) / 100);

        search_limits.budget.total_ms = total;
        search_limits.budget.soft_ms = soft;
        search_limits.budget.hard_ms = std::max(hard, soft);
    }

    search_limits.budget.start_ms = TimeManager::now_ms();
    search_limits.budget.soft_deadline_ms =
        (search_limits.budget.soft_ms > 0)
            ? (search_limits.budget.start_ms + search_limits.budget.soft_ms)
            : 0;

    search_limits.budget.hard_deadline_ms =
        (search_limits.budget.hard_ms > 0)
            ? (search_limits.budget.start_ms + search_limits.budget.hard_ms)
            : 0;
}

} // namespace search::time