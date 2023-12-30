//
// Created by andrew on 12/28/23.
//

#pragma once

#include "io/skizzay/identigen/is_template.h"

#include <chrono>
#include <type_traits>

namespace io::skizzay::identigen {
   template<typename T>
   using is_timestamp = is_template<std::chrono::time_point, T>;

   template<typename T>
   concept timestamp = is_timestamp<T>::value;

   template<typename T>
   concept timestamp_provider = std::invocable<T> && timestamp<std::invoke_result_t<T> >;
} // io::skizzay::identigen
