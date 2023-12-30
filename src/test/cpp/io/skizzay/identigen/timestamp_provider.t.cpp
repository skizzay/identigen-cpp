//
// Created by andrew on 12/28/23.
//

#include <catch2/catch_all.hpp>
#include <io/skizzay/identigen/timestamp_provider.h>

using namespace io::skizzay::identigen;

namespace {
   constexpr auto expected_timestamp_provider = [] {
      return std::chrono::system_clock::now();
   };
   constexpr auto not_expected_timestamp_provider = [] {};
}

TEST_CASE("timestamp is a time_point", "[timestamp]") {
   REQUIRE(timestamp<std::chrono::system_clock::time_point>);
}

TEST_CASE("timestamp_provider returns a timestamp", "[timestamp_provider]") {
   REQUIRE(timestamp_provider<decltype(expected_timestamp_provider)>);
   REQUIRE_FALSE(timestamp_provider<decltype(not_expected_timestamp_provider)>);
}