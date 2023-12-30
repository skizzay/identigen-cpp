//
// Created by andrew on 12/28/23.
//

#include <io/skizzay/identigen/value_provider.h>
#include <catch2/catch_all.hpp>

using namespace io::skizzay::identigen;

namespace {
   struct missing_value final {
      [[nodiscard]]
      constexpr std::size_t num_significant_bits() const noexcept {
         return 0;
      }
   };

   struct missing_num_significant_bits final {
      [[nodiscard]]
      constexpr std::size_t value() const noexcept {
         return 0;
      }
   };

   struct expected_value_provider final {
      [[nodiscard]]
      constexpr std::size_t value(auto const, auto const) const noexcept {
         return 0;
      }

      [[nodiscard]]
      constexpr std::size_t num_significant_bits() const noexcept {
         return 0;
      }
   };
}

TEST_CASE("value_provider interface", "[value_provider]") {
   REQUIRE(value_provider<expected_value_provider>);
   REQUIRE(value_provider<missing_value>);
   REQUIRE_FALSE(value_provider<missing_num_significant_bits>);
}

TEST_CASE("value_provider_for interface", "[value_provider]") {
   REQUIRE(value_provider_for<expected_value_provider, std::chrono::system_clock::time_point, int>);
   REQUIRE_FALSE(value_provider_for<missing_value, std::chrono::system_clock::time_point, int>);
   REQUIRE_FALSE(value_provider_for<missing_num_significant_bits, std::chrono::system_clock::time_point, int>);
}

TEST_CASE("value_provider_utilities calculates significant bits", "[value_provider]") {
   REQUIRE(value_provider_utilities::calculate_num_significant_bits(1) == 1);
   REQUIRE(value_provider_utilities::calculate_num_significant_bits(1 << 1) == 2);
   REQUIRE(value_provider_utilities::calculate_num_significant_bits(1 << 2) == 3);
   REQUIRE(value_provider_utilities::calculate_num_significant_bits(1 << 6) == 7);
}

TEST_CASE("value_provider_utilities from_constant", "[value_provider]") {
   constexpr auto provider = value_provider_utilities::from_constant(42);
   REQUIRE(provider.value(std::chrono::system_clock::now(), 1) == 42);
   REQUIRE(provider.num_significant_bits() == 6);
}

TEST_CASE("value_provider_utilities partitioned", "[value_provider]") {
   constexpr auto provider = value_provider_utilities::partitioned(11);
   REQUIRE(provider.value(std::chrono::system_clock::now(), 1) == 1);
   REQUIRE(provider.value(std::chrono::system_clock::now(), 13) == 2);
   REQUIRE(provider.num_significant_bits() == 4);
}

TEST_CASE("value_provider_utilities from_timestamp", "[value_provider]") {
   using namespace std::chrono;
   auto const today = sys_days{std::chrono::floor<days>(system_clock::now())};
   auto const ts = today + hours{3};
   auto const provider = value_provider_utilities::from_timestamp(time_point_cast<milliseconds>(today),
                                                                  duration_cast<milliseconds>(days{1}));
   auto const expected = duration_cast<milliseconds>(ts - today).count();
   REQUIRE(provider.value(ts, 1) == expected);
   REQUIRE(provider.num_significant_bits() == 27);
}
