//
// Created by Marvin Becker on 18.03.24.
//

/**
 * @file board_exception.h
 * @brief Custom exception type for interrupting board operations.
 *
 * This exception is used inside the chess engine to abort long-running
 * board-related computations (for example search) in a controlled way,
 * while still providing an explanatory message.
 */

#pragma once

#include <exception>
#include <string>
#include <utility>

/**
 * @brief Exception signaling an external interrupt in the board logic.
 *
 * Typical use cases include stopping the search when a piece is
 * missing.
 */
class BoardInterruptException final : public std::exception {
public:

    /**
     * @brief Constructs an interrupt exception with a message.
     *
     * @param msg Human-readable description of the reason for the interrupt.
     */
    explicit BoardInterruptException(std::string msg) : message{std::move(msg)} {
    }

    /**
     * @brief Returns the explanatory string.
     *
     * Overrides std::exception::what() to return the stored message.
     *
     * @return Null-terminated C string with the error description.
     */
    [[nodiscard]] const char* what() const noexcept override { return message.c_str(); }

private:
    std::string message;
};
