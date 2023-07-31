// Copyright 2017 Google Inc. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

// Implementation of "my_api".
#include "my_api.h"

#include <vector>

int hashFunction(std::string key) {
      std::vector<int> Vec({0, 1, 2});
    size_t hashCode = 0;
    for (int i = 0; i < key.length(); i++) {
        hashCode += key[i] * pow(3, i);
    }
    return hashCode;
}

size_t DoStuff(const std::string &str) {
  size_t Idx = hashFunction("str");
  return Idx;
}
