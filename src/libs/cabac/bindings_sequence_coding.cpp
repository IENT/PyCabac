#include <cstdint>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>

#include "sequence_encoder.h"
#include "sequence_decoder.h"
#include "CommonDef.h"
#include "binarization.h"
#include "context_selector.h"

namespace py = pybind11;

void init_pybind_sequence_coding(py::module &m) {
    // ---------------------------------------------------------------------------------------------------------------------
    // SequenceEncoder
    py::class_<cabacSimpleSequenceEncoder, cabacSymbolEncoder>(m, "cabacSimpleSequenceEncoder")
        .def(py::init<>())
        .def("encodeSymbolsBypass", [](cabacSimpleSequenceEncoder &self, const py::array_t<uint64_t> &symbols, 
            binarization::BinarizationId binId, const std::vector<unsigned int> binParams,
            const py::array_t<bool> &mask          
        ) {
            // Request buffer info for symbols and mask
            py::buffer_info bufSymbols = symbols.request();
            uint64_t *ptrSymbols = static_cast<uint64_t *>(bufSymbols.ptr);

            py::buffer_info bufMask = mask.request();
            bool *ptrMask = static_cast<bool *>(bufMask.ptr);
            
            // Check that symbols and mask (if given) have the same size
            if (bufMask.size > 0 && bufSymbols.size != bufMask.size) {
                throw std::runtime_error("Symbols and mask must have the same size");
            }
            self.encodeSymbolsBypass(
                ptrSymbols, bufSymbols.size, binId, binParams, ptrMask, bufMask.size
            );
        }, "Bypass-encode a sequence of symbols", 
            py::arg("symbols"), py::arg("binId"), py::arg("binParams"),
            py::arg("mask")=py::array_t<bool>{}
        )
        .def("encodeSymbols", [](cabacSimpleSequenceEncoder &self, const py::array_t<uint64_t> &symbols, 
            binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId,
            const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams, 
            std::vector<unsigned int> prevSymbolOffsets, py::array_t<bool> &mask
        ) {
            py::buffer_info bufSymbols = symbols.request();
            uint64_t *ptrSymbols = static_cast<uint64_t *>(bufSymbols.ptr);

            py::buffer_info bufMask = mask.request();
            bool *ptrMask = static_cast<bool *>(bufMask.ptr);

            // Check that symbols and mask have the same size
            if (bufMask.size > 0 && bufSymbols.size != bufMask.size) {
                throw std::runtime_error("Symbols and mask must have the same size, symbols.size: " + std::to_string(bufSymbols.size) +
                                         ", mask.size: " + std::to_string(bufMask.size));
            }
            self.encodeSymbols(
                ptrSymbols, bufSymbols.size, binId, ctxModelId, binParams, ctxParams, prevSymbolOffsets, ptrMask, bufMask.size
            );
        }, "Context-encode a sequence of symbols", 
            py::arg("symbols"), py::arg("binId"), py::arg("ctxModelId"), py::arg("binParams"), py::arg("ctxParams"), 
            py::arg("prevSymbolOffsets")=std::vector<unsigned int>{}, py::arg("mask")=py::array_t<bool>{}
        )
        .def("encodeSymbolBypass", [](cabacSimpleSequenceEncoder &self, const uint64_t symbol,
            binarization::BinarizationId binId, const std::vector<unsigned int> binParams
        ) {
            self.encodeSymbolBypass(symbol, binId, binParams);
        })
        .def("encodeSymbol", [](cabacSimpleSequenceEncoder &self, const uint64_t symbol, const unsigned int d, const py::array_t<uint64_t> &symbolsPrev,
            binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId,
            const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams
        ) {
            py::buffer_info buf = symbolsPrev.request();
            uint64_t *ptr = static_cast<uint64_t *>(buf.ptr);
            self.encodeSymbol(symbol, d, ptr, binId, ctxModelId, binParams, ctxParams);
        })
        .def("encodeSymbol", [](cabacSimpleSequenceEncoder &self, const uint64_t symbol, const py::array_t<uint64_t> &symbolsPrev,
            binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId,
            const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams
        ) {
            py::buffer_info buf = symbolsPrev.request();
            uint64_t *ptr = static_cast<uint64_t *>(buf.ptr);
            self.encodeSymbol(symbol, 0, ptr, binId, ctxModelId, binParams, ctxParams);
        });


    // ---------------------------------------------------------------------------------------------------------------------
    // SequenceDecoder
    py::class_<cabacSimpleSequenceDecoder, cabacSymbolDecoder>(m, "cabacSimpleSequenceDecoder")
        .def(py::init<std::vector<uint8_t>>())
        .def("decodeSymbolsBypass", [](cabacSimpleSequenceDecoder &self, unsigned int numSymbols,
            binarization::BinarizationId binId, const std::vector<unsigned int> binParams,
            py::array_t<bool> &mask
        ) {
            // Allocate memory for symbols
            auto symbols = py::array_t<uint64_t>(numSymbols);

            // Request buffer info for symbols and mask
            py::buffer_info bufSymbols = symbols.request();
            uint64_t *ptrSymbols = static_cast<uint64_t *>(bufSymbols.ptr);

            py::buffer_info bufMask = mask.request();
            bool *ptrMask = static_cast<bool *>(bufMask.ptr);

            // Check that symbols and mask have the same size
            if (bufMask.size > 0 && bufSymbols.size != bufMask.size) {
                throw std::runtime_error("Symbols and mask must have the same size");
            }

            self.decodeSymbolsBypass(
                ptrSymbols, numSymbols, binId, binParams, ptrMask, bufMask.size
            );

            return symbols;
        }, "Bypass-decode a sequence of symbols", 
            py::arg("numSymbols"), py::arg("binId"), py::arg("binParams"), 
            py::arg("mask")=py::array_t<bool>{}
        )
        .def("decodeSymbols", [](cabacSimpleSequenceDecoder &self, unsigned int numSymbols,
            binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId,
            const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams,
            std::vector<unsigned int> prevSymbolOffsets, py::array_t<bool> &mask
        ) {
            
            auto symbols = py::array_t<uint64_t>(numSymbols);

            py::buffer_info bufSymbols = symbols.request();
            uint64_t *ptrSymbols = static_cast<uint64_t *>(bufSymbols.ptr);

            py::buffer_info bufMask = mask.request();
            bool *ptrMask = static_cast<bool *>(bufMask.ptr);

            // Check that symbols and mask have the same size
            if (bufMask.size > 0 && bufSymbols.size != bufMask.size) {
                throw std::runtime_error("Symbols and mask must have the same size");
            }

            self.decodeSymbols(ptrSymbols, numSymbols, binId, ctxModelId, binParams, ctxParams, prevSymbolOffsets, ptrMask, bufMask.size);

            return symbols;
        }, "Context-decode a sequence of symbols", 
            py::arg("numSymbols"), py::arg("binId"), py::arg("ctxModelId"), py::arg("binParams"), py::arg("ctxParams"), py::arg("prevSymbolOffsets")=std::vector<unsigned int>{},
            py::arg("mask")=py::array_t<bool>{}
        )
        .def("decodeSymbolBypass", [](cabacSimpleSequenceDecoder &self, 
            binarization::BinarizationId binId, const std::vector<unsigned int> binParams
        ) {
            return self.decodeSymbolBypass(binId, binParams);
        })
        .def("decodeSymbol", [](cabacSimpleSequenceDecoder &self, const unsigned int d, const py::array_t<uint64_t> &symbolsPrev,
            binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId,
            const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams
        ) {
            
            py::buffer_info buf = symbolsPrev.request();
            uint64_t *ptr = static_cast<uint64_t *>(buf.ptr);

            return self.decodeSymbol(d, ptr, binId, ctxModelId, binParams, ctxParams);
        })
        .def("decodeSymbol", [](cabacSimpleSequenceDecoder &self, const py::array_t<uint64_t> &symbolsPrev,
            binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId,
            const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams
        ) {
            
            py::buffer_info buf = symbolsPrev.request();
            uint64_t *ptr = static_cast<uint64_t *>(buf.ptr);

            return self.decodeSymbol(0, ptr, binId, ctxModelId, binParams, ctxParams);
        });

}  // init_pybind_sequence_coding