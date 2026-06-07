//
// Created by Marvin Becker on 18.03.24.
//

/**
 * @file fen_exception.h
 * @brief Exception type for invalid FEN (Forsyth–Edwards Notation) strings.
 *
 * This exception is thrown when the engine encounters malformed or
 * semantically invalid FEN input while parsing a board state. It provides
 * a clear message for debugging and GUI error reporting.
 */

#pragma once

#include <exception>
#include <string>
#include <utility>

/**
 * @brief Exception representing an invalid or unparsable FEN string.
 *
 * Used by the FEN parser to signal formatting errors, wrong field counts,
 * illegal characters, impossible board states, or inconsistencies in
 * piece placement and metadata.
 */
class InvalidFENException final : public std::exception {
public:

    /**
     * @brief Constructs the exception with a descriptive error message.
     *
     * @param msg A human-readable explanation of why the FEN was invalid.
     */
    explicit InvalidFENException(std::string msg) : message{std::move(msg)} {
    }

    /**
     * @brief Returns the stored error message.
     *
     * Overrides std::exception::what() to return a stable C-string representing
     * the cause of the FEN parsing failure.
     *
     * @return Null-terminated C-string describing the error.
     */
    [[nodiscard]] const char* what() const noexcept override { return message.c_str(); }

private:
    std::string message;
};
