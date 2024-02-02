//
// Created by Harrm on 15.01.2021.
//

#include <cassert>

#include <scale/encode.hpp>
#include <scale/scale.hpp>

int main() {
  auto enc = scale::encode(42).value();
  auto dec = scale::decode<int>(enc).value();
  assert(42 == dec);
}
