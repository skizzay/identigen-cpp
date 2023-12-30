//
// Created by andrew on 12/28/23.
//

#pragma once

#include <limits>
#include <span>
#include <stdexcept>
#include <algorithm>
#include <bit>
#include <utility>
#include <array>
#if __has_include(<ieee754.h>)
#include <ieee754.h>
#endif

namespace io::skizzay::identigen {
   struct buffer_overflow : std::runtime_error {
      using std::runtime_error::runtime_error;
   };

   constexpr inline std::endian float_word_order = []() -> std::endian {
      if constexpr (std::endian::big == std::endian::native) {
         return std::endian::big;
      }
      else {
         // I've spent too much time on this, and I'm not sure if it's even possible to detect the float word order
         // without using preprocessor macros.
         return __FLOAT_WORD_ORDER == __ORDER_BIG_ENDIAN__ ? std::endian::big : std::endian::little;
      }
   }();

   template<std::endian E>
   struct read_buffer final {
      using underlying_type = std::span<std::byte const>;
      using value_type = std::ranges::range_value_t<underlying_type>;
      using element_type = underlying_type::element_type;
      using size_type = std::ranges::range_size_t<underlying_type>;

      struct decoder final {
         friend class read_buffer;

         template<std::integral I>
         constexpr explicit operator I() {
            reader_.validate_read_size(sizeof(I));
            std::remove_cv_t<I> x = {};
            reader_.template decode<I>(x);
            reader_.advance(sizeof(I));
            return x;
         }

         constexpr explicit operator float() const {
            reader_.validate_read_size(sizeof(float));
            std::uint32_t x = {};
            reader_.decode(x);
            reader_.advance(sizeof(float));
            return std::bit_cast<float>(x);
         }

         constexpr explicit operator double() const {
            reader_.validate_read_size(sizeof(double));
            std::array<std::uint32_t, 2> parts = {};
            if constexpr (std::endian::big == std::endian::native || std::endian::big == float_word_order) {
               if constexpr (std::endian::big == E) {
                  reader_.decode(parts[0]);
                  reader_.advance(sizeof(parts[0]));
                  reader_.decode(parts[1]);
                  reader_.advance(sizeof(parts[1]));
               }
               else {
                  reader_.decode(parts[1]);
                  reader_.advance(sizeof(parts[1]));
                  reader_.decode(parts[0]);
                  reader_.advance(sizeof(parts[0]));
               }
            }
            else {
               if constexpr (std::endian::big == E) {
                  reader_.decode(parts[1]);
                  reader_.advance(sizeof(parts[1]));
                  reader_.decode(parts[0]);
                  reader_.advance(sizeof(parts[0]));
               }
               else {
                  reader_.decode(parts[0]);
                  reader_.advance(sizeof(parts[0]));
                  reader_.decode(parts[1]);
                  reader_.advance(sizeof(parts[1]));
               }
            }
            reader_.advance(sizeof(double));
            return std::bit_cast<double>(parts);
         }

         constexpr explicit operator std::span<std::byte const>() {
            auto const recovery_position = reader_.position();
            try {
               auto const n = static_cast<size_type>(*this);
               reader_.validate_read_size(n);
               auto const result = reader_.buffer_.subspan(reader_.position_, n);
               reader_.advance(n);
               return result;
            }
            catch (...) {
               reader_.position_ = recovery_position;
               throw;
            }
         }

      private:
         explicit decoder(read_buffer &reader)
            : reader_(reader) {
         }

         read_buffer &reader_;
      };

      explicit read_buffer(underlying_type const buffer, size_type const offset = {}) noexcept
         : buffer_{buffer},
           position_{offset} {
      }

      [[nodiscard]]
      size_type capacity() const noexcept {
         return buffer_.size();
      }

      [[nodiscard]]
      size_type remaining() const noexcept {
         return capacity() - position_;
      }

      [[nodiscard]]
      size_type position() const noexcept {
         return position_;
      }

      decoder get() noexcept {
         return decoder{*this};
      }

      [[nodiscard]]
      read_buffer subreader(std::size_t const start, std::size_t const length) const {
         validate_subbuffer_size(start, length);
         return read_buffer{buffer_.subspan(start, length)};
      }

      [[nodiscard]]
      read_buffer subreader(std::size_t const start) const {
         return subreader(start, capacity() - start);
      }

      [[nodiscard]]
      read_buffer subreader() const noexcept {
         return subreader(position());
      }

   private:
      void validate_read_size(size_type const size) const {
         if (remaining() < size) {
            throw buffer_overflow{"Cannot read value from buffer, not enough space remaining"};
         }
      }

      void validate_subbuffer_size(size_type const start, size_type const length) const {
         if (capacity() < start + length) {
            throw buffer_overflow{"Cannot create subreader, not enough space remaining"};
         }
         if (start < start + length) {
            throw std::out_of_range{"Cannot create subreader, start + length causes integer overflow"};
         }
      }

      void advance(size_type const n) noexcept {
         position_ += n;
      }

      template<std::integral I>
      void decode(I &x);

      underlying_type buffer_;
      size_type position_;
   };

   template<>
   template<std::integral I>
   void read_buffer<std::endian::little>::decode(I &x) {
      if constexpr (8 == sizeof(I)) {
         x = static_cast<I>(buffer_[position() + 7]) << 56;
         x |= static_cast<I>(buffer_[position() + 6]) << 48;
         x |= static_cast<I>(buffer_[position() + 5]) << 40;
         x |= static_cast<I>(buffer_[position() + 4]) << 32;
         x |= static_cast<I>(buffer_[position() + 3]) << 24;
         x |= static_cast<I>(buffer_[position() + 2]) << 16;
         x |= static_cast<I>(buffer_[position() + 1]) << 8;
         x |= static_cast<I>(buffer_[position()]);
      }
      else if constexpr (4 == sizeof(I)) {
         x = static_cast<I>(buffer_[position() + 3]) << 24;
         x |= static_cast<I>(buffer_[position() + 2]) << 16;
         x |= static_cast<I>(buffer_[position() + 1]) << 8;
         x |= static_cast<I>(buffer_[position()]);
      }
      else if constexpr (2 == sizeof(I)) {
         x = static_cast<I>(buffer_[position() + 1]) << 8;
         x |= static_cast<I>(buffer_[position()]);
      }
      else if constexpr (1 == sizeof(I)) {
         x = static_cast<I>(buffer_[position()]);
      }
      else {
         static_assert(8 == sizeof(I), "Invalid integral size");
      }
   }


   template<>
   template<std::integral I>
   void read_buffer<std::endian::big>::decode(I &x) {
      if constexpr (1 == sizeof(I)) {
         x = static_cast<I>(buffer_[position()]);
      }
      else if constexpr (2 == sizeof(I)) {
         x = static_cast<I>(buffer_[position()]) << 8;
         x |= static_cast<I>(buffer_[position() + 1]);
      }
      else if constexpr (4 == sizeof(I)) {
         x = static_cast<I>(buffer_[position()]) << 24;
         x |= static_cast<I>(buffer_[position() + 1]) << 16;
         x |= static_cast<I>(buffer_[position() + 2]) << 8;
         x |= static_cast<I>(buffer_[position() + 3]);
      }
      else if constexpr (8 == sizeof(I)) {
         x = static_cast<I>(buffer_[position()]) << 56;
         x |= static_cast<I>(buffer_[position() + 1]) << 48;
         x |= static_cast<I>(buffer_[position() + 2]) << 40;
         x |= static_cast<I>(buffer_[position() + 3]) << 32;
         x |= static_cast<I>(buffer_[position() + 4]) << 24;
         x |= static_cast<I>(buffer_[position() + 5]) << 16;
         x |= static_cast<I>(buffer_[position() + 6]) << 8;
         x |= static_cast<I>(buffer_[position() + 7]);
      }
      else {
         static_assert(8 == sizeof(I), "Invalid integral size");
      }
   }


   template<std::endian E>
   struct write_buffer final {
      using underlying_type = std::span<std::byte>;
      using value_type = std::ranges::range_value_t<underlying_type>;
      using element_type = underlying_type::element_type;
      using size_type = std::ranges::range_size_t<underlying_type>;

      explicit write_buffer(underlying_type const buffer, size_type const offset = {}) noexcept
         : buffer_{buffer},
           position_{offset} {
      }

      [[nodiscard]]
      size_type capacity() const noexcept {
         return buffer_.size();
      }

      [[nodiscard]]
      size_type remaining() const noexcept {
         return capacity() - position_;
      }

      [[nodiscard]]
      size_type position() const noexcept {
         return position_;
      }

      size_type position(size_type const n) {
         if (capacity() < n) {
            throw std::out_of_range{"Cannot set position, position is greater than capacity"};
         }
         return std::exchange(position_, n);
      }

      write_buffer &put(std::integral auto const x) {
         validate_put_size(sizeof(x));
         encode(x);
         position_ += sizeof(x);
         return *this;
      }

      write_buffer &put(float const x) {
         return put(std::bit_cast<std::uint32_t>(x));
      }

      write_buffer &put(double const x) {
         validate_put_size(sizeof(x));
         auto const parts = std::bit_cast<std::array<std::uint32_t, 2> >(x);
         if constexpr (std::endian::big == std::endian::native || std::endian::big == float_word_order) {
            if constexpr (std::endian::big == E) {
               encode(parts[0]);
               advance(sizeof(parts[0]));
               encode(parts[1]);
               advance(sizeof(parts[1]));
            }
            else {
               encode(parts[1]);
               advance(sizeof(parts[1]));
               encode(parts[0]);
               advance(sizeof(parts[0]));
            }
         }
         else {
            if constexpr (std::endian::big == E) {
               encode(parts[1]);
               advance(sizeof(parts[1]));
               encode(parts[0]);
               advance(sizeof(parts[0]));
            }
            else {
               encode(parts[0]);
               advance(sizeof(parts[0]));
               encode(parts[1]);
               advance(sizeof(parts[1]));
            }
         }
         return *this;
      }

      // template<std::size_t N>
      // write_buffer &put(std::span<std::byte const, N> const bytes) {
      //    mark();
      //    try {
      //       put(bytes.size());
      //       std::ranges::copy(bytes, buffer_.subspan(position_));
      //       position_ += bytes.size();
      //       return *this;
      //    }
      //    catch (...) {
      //       reset();
      //       throw;
      //    }
      // }

      // Force the compiler to use the above overload for std::span<std::byte const>
      template<std::ranges::sized_range R>
      write_buffer &put(R const &range) {
         auto const recovery_position = position();
         try {
            put(std::ranges::size(range));
            if constexpr (is_safe_to_copy<R>()) {
               std::ranges::copy(range, buffer_.subspan(position_));
               advance(std::ranges::size(range));
            }
            else {
               std::ranges::for_each(range, [this](auto const &x) {
                  put(x);
               });
            }
            return *this;
         }
         catch (...) {
            position_ = recovery_position;
            throw;
         }
      }

      [[nodiscard]]
      std::span<std::byte const> to_input_buffer(std::size_t const start, std::size_t const length) const {
         validate_subbuffer_size(start, length);
         return std::as_bytes(buffer_.subspan(start, length));
      }

      [[nodiscard]]
      std::span<std::byte const> to_input_buffer(std::size_t const start) const {
         return to_input_buffer(start, position() - start);
      }

      [[nodiscard]]
      std::span<std::byte const> to_input_buffer() const noexcept {
         return to_input_buffer(0);
      }

      [[nodiscard]]
      write_buffer subwriter(std::size_t const start, std::size_t const length) const {
         validate_subbuffer_size(start, length);
         return write_buffer{buffer_.subspan(start, length)};
      }

      [[nodiscard]]
      write_buffer subwriter(std::size_t const start) const {
         return subwriter(start, capacity() - start);
      }

      [[nodiscard]]
      write_buffer subwriter() const noexcept {
         return subwriter(position());
      }

   private:
      template<typename R>
      static constexpr bool is_safe_to_copy() {
         using value_t = std::remove_cv_t<std::ranges::range_value_t<R>>;
         return std::ranges::contiguous_range<R> && std::is_trivially_copyable_v<value_t> && 1 == sizeof(value_t);
      }

      void validate_put_size(size_type const size) const {
         if (remaining() < size) {
            throw buffer_overflow{"Cannot put value into buffer, not enough space remaining"};
         }
      }

      void validate_subbuffer_size(size_type const start, size_type const length) const {
         if (capacity() < start + length) {
            throw buffer_overflow{"Cannot create subwriter, not enough space remaining"};
         }
         if (start > start + length) {
            throw std::out_of_range{"Cannot create subwriter, start + length causes integer overflow"};
         }
      }

      void advance(size_type const n) noexcept {
         position_ += n;
      }

      template<std::integral I>
      void encode(I x) noexcept;

      underlying_type buffer_;
      size_type position_;
   };

   template<>
   template<std::integral I>
   void write_buffer<std::endian::little>::encode(I const x) noexcept {
      if constexpr (8 == sizeof(I)) {
         buffer_[position() + 7] = static_cast<std::byte>(x >> 56);
         buffer_[position() + 6] = static_cast<std::byte>(x >> 48);
         buffer_[position() + 5] = static_cast<std::byte>(x >> 40);
         buffer_[position() + 4] = static_cast<std::byte>(x >> 32);
         buffer_[position() + 3] = static_cast<std::byte>(x >> 24);
         buffer_[position() + 2] = static_cast<std::byte>(x >> 16);
         buffer_[position() + 1] = static_cast<std::byte>(x >> 8);
         buffer_[position()] = static_cast<std::byte>(x);
      }
      else if constexpr (4 == sizeof(I)) {
         buffer_[position() + 3] = static_cast<std::byte>(x >> 24);
         buffer_[position() + 2] = static_cast<std::byte>(x >> 16);
         buffer_[position() + 1] = static_cast<std::byte>(x >> 8);
         buffer_[position()] = static_cast<std::byte>(x);
      }
      else if constexpr (2 == sizeof(I)) {
         buffer_[position() + 1] = static_cast<std::byte>(x >> 8);
         buffer_[position()] = static_cast<std::byte>(x);
      }
      else if constexpr (1 == sizeof(I)) {
         buffer_[position()] = static_cast<std::byte>(x);
      }
      else {
         static_assert(8 == sizeof(I), "Invalid integral size");
      }
   }


   template<>
   template<std::integral I>
   void write_buffer<std::endian::big>::encode(I const x) noexcept {
      if constexpr (1 == sizeof(I)) {
         buffer_[position()] = static_cast<std::byte>(x);
      }
      else if constexpr (2 == sizeof(I)) {
         buffer_[position()] = static_cast<std::byte>(x >> 8);
         buffer_[position() + 1] = static_cast<std::byte>(x);
      }
      else if constexpr (4 == sizeof(I)) {
         buffer_[position()] = static_cast<std::byte>(x >> 24);
         buffer_[position() + 1] = static_cast<std::byte>(x >> 16);
         buffer_[position() + 2] = static_cast<std::byte>(x >> 8);
         buffer_[position() + 3] = static_cast<std::byte>(x);
      }
      else if constexpr (8 == sizeof(I)) {
         buffer_[position()] = static_cast<std::byte>(x >> 56);
         buffer_[position() + 1] = static_cast<std::byte>(x >> 48);
         buffer_[position() + 2] = static_cast<std::byte>(x >> 40);
         buffer_[position() + 3] = static_cast<std::byte>(x >> 32);
         buffer_[position() + 4] = static_cast<std::byte>(x >> 24);
         buffer_[position() + 5] = static_cast<std::byte>(x >> 16);
         buffer_[position() + 6] = static_cast<std::byte>(x >> 8);
         buffer_[position() + 7] = static_cast<std::byte>(x);
      }
      else {
         static_assert(8 == sizeof(I), "Invalid integral size");
      }
   }
} // io::skizzay::identigen
