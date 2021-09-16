#include <pybind11/pybind11.h>
#include "math.h"

namespace py = pybind11;

PYBIND11_MODULE(pybind11_example, m) {
    m.doc() = "pybind11 example";

    m.def("add", &add, py::arg("i"), py::arg("j"));
    m.def("subtract", &subtract, py::arg("i"), py::arg("j"));
}
