//
// Created by andrew on 12/28/23.
//

#pragma once

#include <concepts>
#include <functional>

namespace io::skizzay::identigen {
   template<typename T>
   concept key = std::equality_comparable<T> && requires(T t, std::hash<T> h) {
      { h(t) } -> std::same_as<std::size_t>;
   };

   template<typename T>
   concept sortable_key = key<T> && std::totally_ordered<T>;
}
