//
// Created by andrew on 12/28/23.
//

#pragma once

#include "io/skizzay/identigen/timestamp_provider.h"
#include "io/skizzay/identigen/key.h"

#include <concepts>

namespace io::skizzay::identigen {
   template<typename T>
   concept value_provider = requires(T t) {
      { t.num_significant_bits() } -> std::same_as<std::size_t>;
   };

   template<typename T, typename Timestamp, typename Key>
   concept value_provider_for = value_provider<T> && timestamp<Timestamp> && key<Key> && requires(
                             T t, Timestamp ts, Key k
                          ) {
                                   { t.value(ts, k) } -> std::same_as<std::size_t>;
                                };

   struct value_provider_utilities {
      value_provider_utilities() = delete;

      static constexpr std::size_t calculate_num_significant_bits(std::size_t value) noexcept {
         std::size_t result = 0;
         while (value > 0) {
            ++result;
            value >>= 1;
         }
         return result;
      }

      static constexpr value_provider auto from_constant(std::size_t const value) noexcept {
         return constant_value_provider{value, calculate_num_significant_bits(value)};
      }

      static constexpr value_provider auto partitioned(std::size_t const num_buckets) noexcept {
         return key_value_provider{num_buckets, calculate_num_significant_bits(num_buckets - 1)};
      }

      template<timestamp T>
      static constexpr value_provider auto from_timestamp(T const epoch,
                                                          typename T::duration const max_duration) noexcept {
         return timestamp_value_provider{epoch, max_duration, calculate_num_significant_bits(max_duration.count() - 1)};
      }

   private:
      struct constant_value_provider final {
         std::size_t const x;
         std::size_t const significant_bits;

         [[nodiscard]]
         constexpr std::size_t value(timestamp auto const, key auto const) const noexcept {
            return x;
         }

         [[nodiscard]]
         constexpr std::size_t num_significant_bits() const noexcept {
            return significant_bits;
         }
      };

      struct key_value_provider final {
         std::size_t const num_buckets;
         std::size_t const significant_bits;

         [[nodiscard]]
         constexpr std::size_t value(timestamp auto const, key auto const k) const noexcept {
            return std::hash<std::remove_cv_t<decltype(k)>>{}(k) % num_buckets;
         }

         [[nodiscard]]
         constexpr std::size_t num_significant_bits() const noexcept {
            return significant_bits;
         }
      };

      template<timestamp T>
      struct timestamp_value_provider {
         T const epoch;
         typename T::duration const max_duration;
         std::size_t const significant_bits;

         template<timestamp U>
         requires std::same_as<typename T::clock, typename U::clock>
         [[nodiscard]]
         constexpr std::size_t value(U ts, key auto const &) const noexcept {
            auto const diff = ts - epoch;
            return static_cast<std::size_t>(diff.count() % max_duration.count());
         }

         [[nodiscard]]
         constexpr std::size_t num_significant_bits() const noexcept {
            return significant_bits;
         }
      };
   };
} // io::skizzay::identigen
