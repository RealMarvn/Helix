//
// Created by Marvin Becker on 18.03.24.
//

#pragma once

#include <exception>
#include <string>
#include <utility>

class BoardInterruptException final : public std::exception {
public:
  explicit BoardInterruptException(std::string msg) : message{std::move(msg)} {
  }

  [[nodiscard]] const char* what() const noexcept override { return message.c_str(); }

private:
  std::string message;
};
