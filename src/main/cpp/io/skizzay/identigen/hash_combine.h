//
// Created by andrew on 12/28/23.
//

#pragma once

#include <functional>

namespace io::skizzay::identigen {
   constexpr std::size_t hash_combine(std::size_t const seed, std::size_t const value) noexcept {
      return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
   }

   template<std::same_as<std::size_t>... H>
   constexpr std::size_t hash_combine(std::size_t const seed, std::size_t const value, H const ... values) noexcept {
      return hash_combine(hash_combine(seed, value), values...);
   }
}