//
// Created by andrew on 12/28/23.
//

#include <catch2/catch_all.hpp>

#include <io/skizzay/identigen/is_template.h>

using namespace io::skizzay::identigen;

TEST_CASE("is_template matches as type trait and concept", "[is_template]") {
   REQUIRE(is_template<std::basic_string, std::string>::value);
   REQUIRE(of_template<std::basic_string, std::string>);
   REQUIRE_FALSE(is_template<std::vector, std::string>::value);
   REQUIRE_FALSE(of_template<std::vector, std::string>);
}