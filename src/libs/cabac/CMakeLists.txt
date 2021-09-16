set(source_files
        bin_decoder.cpp
        bin_encoder.cpp
        bitstream.cpp
        contexts.cpp
)

add_library(cabac_internal ${source_files})
add_library(cabac_pybind11_internal ${source_files})
pybind11_add_module(cabac ${source_files} bindings.cpp)
