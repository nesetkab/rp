#pragma once

class Terminal {
public:
  Terminal();
  ~Terminal();
  Terminal(const Terminal &) = delete;
  Terminal &operator=(const Terminal &) = delete;
};
