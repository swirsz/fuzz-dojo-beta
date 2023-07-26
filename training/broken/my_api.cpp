// Copyright 2017 Google Inc. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

// Implementation of "my_api".
#include "my_api.h"

#include <vector>

// Do some computations with 'str', return the result.
// This function contains a bug. Can you spot it?
int hashFunction(string key) {
    size_t hashCode = 0;
    for (int i = 0; i < key.length(); i++) {
        hashCode += key[i] * pow(3, i);
    }
    return hashCode;
}

size_t DoStuff(const std::string &str) {
  std::vector<int> Vec({0, 1, 2, 3, 4});
  size_t Idx = hashFunction("str")
  if (Idx == 0)
    return Vec[Idx];
  if (Idx == 1)
    return Vec[Idx];
  if (Idx == 2)
    return Vec[Idx];
  return Vec[Idx];
}
