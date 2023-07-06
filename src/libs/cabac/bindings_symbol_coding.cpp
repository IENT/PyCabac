#include <cstdint>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>

#include "symbol_encoder.h"
#include "symbol_decoder.h"
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
        .def("encodeBinsBIbypass", static_cast<void (cabacSymbolEncoder::*)(uint64_t, const unsigned int)>(&cabacSymbolEncoder::encodeBinsBIbypass))
        .def("encodeBinsBI", [](cabacSymbolEncoder &self, uint64_t symbol, const py::array_t<unsigned int>& ctxIdsNumpy, const unsigned int numBins) {
            
            auto buf = ctxIdsNumpy.request();
            unsigned int *ptr = static_cast<unsigned int *>(buf.ptr);
            // std::vector<unsigned int> ctxIds(ptr, ptr + buf.size); // this is slow
            self.encodeBinsBI(symbol, ptr, numBins);
        })
        .def("encodeBinsBi", [](cabacSymbolEncoder &self, uint64_t symbol, CtxFunction &ctxFun, const unsigned int numBins) {
            self.encodeBinsBI(symbol, ctxFun, numBins);
        })
        .def("encodeBinsTUbypass", static_cast<void (cabacSymbolEncoder::*)(uint64_t, const unsigned int)>(&cabacSymbolEncoder::encodeBinsTUbypass),
         "TU-binarize and and bypass-encode symbol",
            py::arg("symbol"), py::arg("numMaxBins")=512)
        .def("encodeBinsTU", [](cabacSymbolEncoder &self, uint64_t symbol, const py::array_t<unsigned int>& ctxIdsNumpy, const unsigned int numMaxVal) {
            
            auto buf = ctxIdsNumpy.request();
            unsigned int *ptr = static_cast<unsigned int *>(buf.ptr);
            self.encodeBinsTU(symbol, ptr, numMaxVal);
        })
        .def("encodeBinsTU", [](cabacSymbolEncoder &self, uint64_t symbol, CtxFunction &ctxFun, const unsigned int numMaxVal) {
            self.encodeBinsTU(symbol, ctxFun, numMaxVal);
        })
        /*.def("encodeBinsTUtest", [](cabacSymbolEncoder &self, uint64_t symbol, const py::array_t<unsigned int>& ctxIdsNumpy, const unsigned int numMaxVal) {
            
            auto buf = ctxIdsNumpy.request();
            unsigned int *ptr = static_cast<unsigned int *>(buf.ptr);

            CtxFunction ctxFun = [&ptr](unsigned int i) {
                return ptr[i];
            };

            self.encodeBinsTU(symbol, ctxFun, numMaxVal);
        })*/
        .def("encodeBinsEGkbypass", static_cast<void (cabacSymbolEncoder::*)(uint64_t, const unsigned int)>(&cabacSymbolEncoder::encodeBinsEGkbypass))
        .def("encodeBinsEGk", [](cabacSymbolEncoder &self, uint64_t symbol, const unsigned int k,  const py::array_t<unsigned int>& ctxIdsNumpy) {
            
            auto buf = ctxIdsNumpy.request();
            unsigned int *ptr = static_cast<unsigned int *>(buf.ptr);
            self.encodeBinsEGk(symbol, k, ptr);
        })
        .def("encodeBinsEGk", [](cabacSymbolEncoder &self, uint64_t symbol, const unsigned int k,  CtxFunction &ctxFun) {
            self.encodeBinsEGk(symbol, k, ctxFun);
        });

    // ---------------------------------------------------------------------------------------------------------------------
    // SymbolDecoder
    py::class_<cabacSymbolDecoder, cabacDecoder>(m, "cabacSymbolDecoder")
        .def(py::init<std::vector<uint8_t>>())
        .def("decodeBinsBIbypass", static_cast<uint64_t (cabacSymbolDecoder::*)(const unsigned int)>(&cabacSymbolDecoder::decodeBinsBIbypass))
        .def("decodeBinsBI", [](cabacSymbolDecoder &self, const py::array_t<unsigned int>& ctxIdsNumpy, const unsigned int numBins) {
            
            auto buf = ctxIdsNumpy.request();
            unsigned int *ptr = static_cast<unsigned int *>(buf.ptr);
            // std::vector<unsigned int> ctxIds(ptr, ptr + buf.size); // this is slow
            return self.decodeBinsBI(ptr, numBins);
        })
        .def("decodeBinsBI", [](cabacSymbolDecoder &self, CtxFunction &ctxFun, const unsigned int numBins) {
            return self.decodeBinsBI(ctxFun, numBins);
        })
        .def("decodeBinsTUbypass", static_cast<uint64_t (cabacSymbolDecoder::*)(const unsigned int)>(&cabacSymbolDecoder::decodeBinsTUbypass),
            "bypass-decode TU-encoded symbol", py::arg("numMaxBins")=512)
        .def("decodeBinsTU", [](cabacSymbolDecoder &self, const py::array_t<unsigned int>& ctxIdsNumpy, const unsigned int numMaxVal) {
            
            auto buf = ctxIdsNumpy.request();
            unsigned int *ptr = static_cast<unsigned int *>(buf.ptr);
            return self.decodeBinsTU(ptr, numMaxVal);
        })
        .def("decodeBinsTU", [](cabacSymbolDecoder &self, CtxFunction &ctxFun, const unsigned int numMaxVal) {
            return self.decodeBinsTU(ctxFun, numMaxVal);
        })
        .def("decodeBinsEGkbypass", static_cast<uint64_t (cabacSymbolDecoder::*)(const unsigned int)>(&cabacSymbolDecoder::decodeBinsEGkbypass))
        .def("decodeBinsEGk", [](cabacSymbolDecoder &self, const unsigned int k, const py::array_t<unsigned int>& ctxIdsNumpy) {
            
            auto buf = ctxIdsNumpy.request();
            unsigned int *ptr = static_cast<unsigned int *>(buf.ptr);
            return self.decodeBinsEGk(k, ptr);
        })
        .def("decodeBinsEGk", [](cabacSymbolDecoder &self, const unsigned int k, CtxFunction &ctxFun) {
            return self.decodeBinsEGk(k, ctxFun);
        });

}  // init_pybind_symbol_coding