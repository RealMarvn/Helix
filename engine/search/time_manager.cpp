//
// Created by Marvin Becker on 16.12.25.
//

#include "time_manager.h"

namespace search::time
{

int TimeManager::default_reserve_ms(const int my_time_ms)
{
    // ~2% reserve, min 100ms
    return std::max(100, my_time_ms / 50);
}

int TimeManager::default_moves_left(const int movestogo)
{
    // If UCI gives no movestogo -> standard estimate
    return (movestogo > 0) ? movestogo : 30;
}

TimeBudget TimeManager::compute_budget_ms(const int side_to_move, const UciTimeControl& tc)
{
    // White = 0 | Black = 1
    const int my_time = (side_to_move == 0) ? tc.wtime : tc.btime;
    const int my_inc = (side_to_move == 0) ? tc.winc : tc.binc;

    // Fallback if no clock infos
    if (my_time <= 0)
    {
        return TimeBudget{
            1400, // soft
            1900, // hard
            2000  // total
        };
    }

    const int reserve = (tc.reserve_ms > 0) ? tc.reserve_ms : default_reserve_ms(my_time);

    // Time, we can only use to calculate
    int safe_time = my_time - tc.overhead_ms - reserve;
    if (safe_time < 0)
        safe_time = 0;

    const int moves_left = default_moves_left(tc.movestogo);

    // Basisbudget
    int budget = safe_time / std::max(1, moves_left);

    // Increment aggressively
    if (my_inc > 0)
        budget += (my_inc * 8) / 10; // 80%

    // Never more than 25% of the rest-time
    const int hard_cap = std::max(10, safe_time / 4);
    budget = std::clamp(budget, 10, hard_cap);

    const int soft_ms = std::max(1, (budget * 70) / 100);
    const int hard_ms = std::max(1, (budget * 95) / 100);

    return TimeBudget{soft_ms, hard_ms, budget};
}

} // namespace search::time