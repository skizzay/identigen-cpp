//
// Created by andrew on 12/28/23.
//

#pragma once

#include <type_traits>

namespace io::skizzay::identigen {
   template<template<typename...> typename T, typename U>
   struct is_template : std::false_type {
   };

   template<template<typename...> typename T, typename... Args>
   struct is_template<T, T<Args...> > : std::true_type {
   };

   template<template<typename...> typename T, typename U>
   concept of_template = is_template<T, U>::value;
} // io::skizzay::identigen
