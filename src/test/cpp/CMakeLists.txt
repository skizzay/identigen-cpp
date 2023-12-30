find_package(Catch2 REQUIRED)

add_executable(identigen_unit_tests
        io/skizzay/identigen/is_template.t.cpp
        io/skizzay/identigen/timestamp_provider.t.cpp
        io/skizzay/identigen/value_provider.t.cpp
        io/skizzay/identigen/buffer.t.cpp
)
target_link_libraries(identigen_unit_tests
        PRIVATE
        identigen-core
        Catch2::Catch2WithMain)