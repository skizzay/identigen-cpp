add_library(identigen-core INTERFACE
        io/skizzay/identigen/key.h
        io/skizzay/identigen/hash_combine.h
        io/skizzay/identigen/buffer.h
)
target_include_directories(identigen-core INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>)
target_sources(identigen-core INTERFACE
        io/skizzay/identigen/is_template.h
        io/skizzay/identigen/timestamp_provider.h
        io/skizzay/identigen/value_provider.h)