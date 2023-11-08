#include <cstdint>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "bin_encoder.h"
#include "bin_decoder.h"
#include "CommonDef.h"

// This would make a simple call of encodeBins*() much faster
// However, ctx_idx = VectorUnsignedInt(ctxIds) would be needed which is slow again.
// cabac.encodeBins*(bin, VectorUnsignedInt(ctxIDs), numMaxBins) would be needed
// 
//PYBIND11_MAKE_OPAQUE(std::vector<unsigned int>);
//
// uncommend line below as well ... py::bind_vector ...

namespace py = pybind11;

void init_pybind_cabac(py::module &);
void init_pybind_symbol_coding(py::module &);
void init_pybind_sequence_coding(py::module &);
void init_pybind_context_selector(py::module &);

PYBIND11_MODULE(cabac, m) {
    m.doc() = "pybind11 eabac";

    init_pybind_cabac(m);
    init_pybind_symbol_coding(m);
    init_pybind_sequence_coding(m);
    init_pybind_context_selector(m);

}  // PYBIND11_MODULE(cabac, m)
