#include <cstdint>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "bin_encoder.h"
#include "bin_decoder.h"
#include "CommonDef.h"
#include "binarization.h"
#include "context_selector.h"

// This would make a simple call of encodeBins*() much faster
// However, ctx_idx = VectorUnsignedInt(ctxIds) would be needed which is slow again.
// cabac.encodeBins*(bin, VectorUnsignedInt(ctxIDs), numMaxBins) would be needed
// 
//PYBIND11_MAKE_OPAQUE(std::vector<unsigned int>);
//
// uncommend line below as well ... py::bind_vector ...

namespace py = pybind11;

void init_pybind_symbol_coding(py::module &m) {
   
    // See comment above
    // py::bind_vector<std::vector<unsigned int>>(m, "VectorUnsignedInt", py::buffer_protocol());

    // ---------------------------------------------------------------------------------------------------------------------    
    // SymbolEncoder
    py::class_<cabacSymbolEncoder, cabacEncoder>(m, "cabacSymbolEncoder")
        .def(py::init<>())
        .def("encodeBinsBIbypass", &cabacSymbolEncoder::encodeBinsBIbypass)
        .def("encodeBinsBI", [](cabacSymbolEncoder &self, uint64_t symbol, const py::array_t<unsigned int>& ctxIdsNumpy, const unsigned int numBins) {
            
            auto buf = ctxIdsNumpy.request();
            unsigned int *ptr = static_cast<unsigned int *>(buf.ptr);
            // std::vector<unsigned int> ctxIds(ptr, ptr + buf.size); // this is slow
            self.encodeBinsBI(symbol, ptr, numBins);
        })
        .def("encodeBinsTUbypass", &cabacSymbolEncoder::encodeBinsTUbypass, 
            "TU-binarize and and bypass-encode symbol",
            py::arg("symbol"), py::arg("numMaxBins")=512
        )
        .def("encodeBinsTU", [](cabacSymbolEncoder &self, uint64_t symbol, const py::array_t<unsigned int>& ctxIdsNumpy, const unsigned int numMaxVal) {
            
            auto buf = ctxIdsNumpy.request();
            unsigned int *ptr = static_cast<unsigned int *>(buf.ptr);
            self.encodeBinsTU(symbol, ptr, numMaxVal);
        })
        .def("encodeBinsEG0bypass", &cabacSymbolEncoder::encodeBinsEG0bypass)
        .def("encodeBinsEG0", [](cabacSymbolEncoder &self, uint64_t symbol, const py::array_t<unsigned int>& ctxIdsNumpy) {
            
            auto buf = ctxIdsNumpy.request();
            unsigned int *ptr = static_cast<unsigned int *>(buf.ptr);
            self.encodeBinsEG0(symbol, ptr);
        })
        .def("encodeBinsEGkbypass", &cabacSymbolEncoder::encodeBinsEGkbypass)
        .def("encodeBinsEGk", [](cabacSymbolEncoder &self, uint64_t symbol, const unsigned int k,  const py::array_t<unsigned int>& ctxIdsNumpy) {
            
            auto buf = ctxIdsNumpy.request();
            unsigned int *ptr = static_cast<unsigned int *>(buf.ptr);
            self.encodeBinsEGk(symbol, k, ptr);
        });

    // ---------------------------------------------------------------------------------------------------------------------
    // SymbolDecoder
    py::class_<cabacSymbolDecoder, cabacDecoder>(m, "cabacSymbolDecoder")
        .def(py::init<std::vector<uint8_t>>())
        .def("decodeBinsBIbypass", &cabacSymbolDecoder::decodeBinsBIbypass)
        .def("decodeBinsBI", [](cabacSymbolDecoder &self, const py::array_t<unsigned int>& ctxIdsNumpy, const unsigned int numBins) {
            
            auto buf = ctxIdsNumpy.request();
            unsigned int *ptr = static_cast<unsigned int *>(buf.ptr);
            // std::vector<unsigned int> ctxIds(ptr, ptr + buf.size); // this is slow
            return self.decodeBinsBI(ptr, numBins);
        })
        .def("decodeBinsTUbypass", &cabacSymbolDecoder::decodeBinsTUbypass,
            "bypass-decode TU-encoded symbol", py::arg("numMaxBins")=512)
        .def("decodeBinsTU", [](cabacSymbolDecoder &self, const py::array_t<unsigned int>& ctxIdsNumpy, const unsigned int numMaxVal) {
            
            auto buf = ctxIdsNumpy.request();
            unsigned int *ptr = static_cast<unsigned int *>(buf.ptr);
            return self.decodeBinsTU(ptr, numMaxVal);
        })
        .def("decodeBinsEG0bypass", &cabacSymbolDecoder::decodeBinsEG0bypass)
        .def("decodeBinsEG0", [](cabacSymbolDecoder &self, const py::array_t<unsigned int>& ctxIdsNumpy) {
            
            auto buf = ctxIdsNumpy.request();
            unsigned int *ptr = static_cast<unsigned int *>(buf.ptr);
            return self.decodeBinsEG0(ptr);
        })
        .def("decodeBinsEGkbypass", &cabacSymbolDecoder::decodeBinsEGkbypass)
        .def("decodeBinsEGk", [](cabacSymbolDecoder &self, const unsigned int k, const py::array_t<unsigned int>& ctxIdsNumpy) {
            
            auto buf = ctxIdsNumpy.request();
            unsigned int *ptr = static_cast<unsigned int *>(buf.ptr);
            return self.decodeBinsEGk(k, ptr);
        });

}  // init_pybind_symbol_coding