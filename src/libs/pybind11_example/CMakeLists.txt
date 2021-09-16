set(source_files
	math.cpp
)

add_library(pybind11_example_internal ${source_files})

pybind11_add_module(pybind11_example ${source_files} bindings.cpp)
