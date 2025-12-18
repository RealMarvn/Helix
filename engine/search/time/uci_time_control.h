//
// Created by Marvin Becker on 18.12.25.
//

#pragma once

/**
 * @brief Raw time control information as provided by the UCI protocol.
 *
 * UciTimeControl represents the clock and increment information received
 * from the GUI via the UCI "go" command. The values stored in this
 * structure are not directly used by the search; instead, they are
 * converted into an internal TimeControl/TimeBudget representation
 * before the search starts.
 *
 * All time values are given in milliseconds.
 */
struct UciTimeControl
{
    /**
     * @brief Remaining time for the white side.
     *
     * The value is given in milliseconds. A negative value indicates
     * that no time information for white was provided by the GUI.
     */
    int wtime = -1;

    /**
     * @brief Remaining time for the black side.
     *
     * The value is given in milliseconds. A negative value indicates
     * that no time information for black was provided by the GUI.
     */
    int btime = -1;

    /**
     * @brief Increment added to white's clock after each move.
     *
     * The value is given in milliseconds. A value of zero indicates
     * that no increment is used.
     */
    int winc = 0;

    /**
     * @brief Increment added to black's clock after each move.
     *
     * The value is given in milliseconds. A value of zero indicates
     * that no increment is used.
     */
    int binc = 0;

    /**
     * @brief Number of moves until the next time control.
     *
     * A negative value indicates that the GUI did not provide a
     * movestogo parameter.
     */
    int movestogo = -1;

    /**
     * @brief Safety margin to compensate for UI, IO, and OS jitter.
     *
     * This value is subtracted from the available time before
     * computing the search budget to reduce the risk of flagging.
     */
    int overhead_ms = 20;

    /**
     * @brief Additional time reserve to avoid time forfeits.
     *
     * If the value is less than or equal to zero, a default reserve
     * based on the remaining time is chosen automatically.
     */
    int reserve_ms = -1;
};
