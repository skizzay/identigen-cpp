//
// Created by andrew on 12/28/23.
//

#include <io/skizzay/identigen/buffer.h>
#include <catch2/catch_all.hpp>

using namespace io::skizzay::identigen;

template<typename T>
T create_value() {
   if constexpr (std::integral<T>) {
      return T{-1};
   }
   else if constexpr (std::floating_point<T>) {
      return T{-1.0};
   }
   else {
      return T{};
   }
};

TEMPLATE_TEST_CASE_SIG("write_buffer can be constructed with a span of bytes", "[write_buffer]", ((std::endian E), E),
                       std::endian::little, std::endian::big) {
   std::array<std::byte, 10> buffer{};
   write_buffer<E> writer{buffer};
   REQUIRE(writer.capacity() == 10);
   REQUIRE(writer.remaining() == 10);
   REQUIRE(writer.position() == 0);
}

TEMPLATE_TEST_CASE_SIG("write_buffer moves position after put", "[write_buffer]", ((std::endian E, typename I), E, I),
                       (std::endian::little, std::int8_t), (std::endian::little, std::int16_t),
                       (std::endian::little, std::int32_t), (std::endian::little, std::int64_t),
                       (std::endian::big, std::int8_t), (std::endian::big, std::int16_t),
                       (std::endian::big, std::int32_t), (std::endian::big, std::int64_t)) {
   std::array<std::byte, 2 * sizeof(I) + 1> buffer{};
   write_buffer<E> writer{buffer};
   writer.put(create_value<I>());
   REQUIRE(writer.capacity() == 2 * sizeof(I) + 1);
   REQUIRE(writer.remaining() == sizeof(I) + 1);
   REQUIRE(writer.position() == sizeof(I));
}

TEMPLATE_TEST_CASE_SIG("read_buffer can be constructed with a span of bytes", "[read_buffer]", ((std::endian E), E),
                       std::endian::little, std::endian::big) {
   std::array<std::byte, 10> buffer{};
   read_buffer<E> buffer_view{buffer};
   REQUIRE(buffer_view.capacity() == 10);
   REQUIRE(buffer_view.remaining() == 10);
   REQUIRE(buffer_view.position() == 0);
}

TEMPLATE_TEST_CASE_SIG("read_buffer can read integers from the buffer written by a write_buffer", "[read_buffer,write_buffer]",
                       ((std::endian E, typename I), E, I), (std::endian::little, std::int8_t),
                       (std::endian::little, std::int16_t), (std::endian::little, std::int32_t),
                       (std::endian::little, std::int64_t), (std::endian::big, std::int8_t),
                       (std::endian::big, std::int16_t), (std::endian::big, std::int32_t),
                       (std::endian::big, std::int64_t)) {
   I const expected = create_value<I>();
   std::array<std::byte, 2 * sizeof(I) + 1> buffer{};
   write_buffer<E> writer{buffer};
   writer.put(expected);
   read_buffer<E> reader{writer.to_input_buffer()};
   I const actual = static_cast<I>(reader.get());
   REQUIRE(actual == expected);
}

TEMPLATE_TEST_CASE_SIG("read_buffer can read floating-point decimals from the buffer written by a write_buffer", "[read_buffer,write_buffer]",
                       ((std::endian E, typename F), E, F), (std::endian::little, float),
                       (std::endian::little, double), (std::endian::big, float),
                       (std::endian::big, double)) {
   F const expected = create_value<F>();
   std::array<std::byte, 2 * sizeof(F) + 1> buffer{};
   write_buffer<E> writer{buffer};
   writer.put(expected);
   read_buffer<E> reader{writer.to_input_buffer()};
   F const actual = static_cast<F>(reader.get());
   REQUIRE(actual == expected);
}

TEST_CASE("write_buffer can write in little endian", "[write_buffer]") {
   std::array<std::byte, sizeof(std::uint32_t)> buffer{};
   write_buffer<std::endian::little> writer{buffer};
   writer.put(std::int32_t{0x12345678});
   REQUIRE(buffer[0] == std::byte{0x78});
   REQUIRE(buffer[1] == std::byte{0x56});
   REQUIRE(buffer[2] == std::byte{0x34});
   REQUIRE(buffer[3] == std::byte{0x12});
}

TEST_CASE("write_buffer can write in big endian", "[write_buffer]") {
   std::array<std::byte, sizeof(std::uint32_t)> buffer{};
   write_buffer<std::endian::big> writer{buffer};
   writer.put(std::int32_t{0x12345678});
   REQUIRE(buffer[0] == std::byte{0x12});
   REQUIRE(buffer[1] == std::byte{0x34});
   REQUIRE(buffer[2] == std::byte{0x56});
   REQUIRE(buffer[3] == std::byte{0x78});
}