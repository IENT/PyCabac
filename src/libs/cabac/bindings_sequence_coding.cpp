#include <cstdint>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

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
            binarization::BinarizationId binId, const std::vector<unsigned int> binParams
        ) {
            auto buf = symbols.request();
            uint64_t *ptr = static_cast<uint64_t *>(buf.ptr);
            self.encodeSymbolsBypass(ptr, buf.size, binId, binParams);
        })
        .def("encodeSymbols", [](cabacSimpleSequenceEncoder &self, const py::array_t<uint64_t> &symbols,
            binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId,
            const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams
        ) {
            auto buf = symbols.request();
            uint64_t *ptr = static_cast<uint64_t *>(buf.ptr);
            self.encodeSymbols(ptr, buf.size, binId, ctxModelId, binParams, ctxParams);
        })
        .def("encodeSymbolBypass", [](cabacSimpleSequenceEncoder &self, const uint64_t symbol,
            binarization::BinarizationId binId, const std::vector<unsigned int> binParams
        ) {
            self.encodeSymbolBypass(symbol, binId, binParams);
        })
        .def("encodeSymbol", [](cabacSimpleSequenceEncoder &self, const uint64_t symbol, const py::array_t<uint64_t> &symbolsPrev,
            binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId,
            const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams
        ) {
            auto buf = symbolsPrev.request();
            uint64_t *ptr = static_cast<uint64_t *>(buf.ptr);
            self.encodeSymbol(symbol, ptr, binId, ctxModelId, binParams, ctxParams);
        });


    // ---------------------------------------------------------------------------------------------------------------------
    // SequenceDecoder
    py::class_<cabacSimpleSequenceDecoder, cabacSymbolDecoder>(m, "cabacSimpleSequenceDecoder")
        .def(py::init<std::vector<uint8_t>>())
        .def("decodeSymbolsBypass", [](cabacSimpleSequenceDecoder &self, unsigned int numSymbols,
            binarization::BinarizationId binId, const std::vector<unsigned int> binParams
        ) {
            auto symbols = py::array_t<uint64_t>(numSymbols);

            py::buffer_info buf = symbols.request();
            uint64_t *symbols_ptr = static_cast<uint64_t *>(buf.ptr);

            self.decodeSymbolsBypass(symbols_ptr, numSymbols, binId, binParams);

            return symbols;
        })
        .def("decodeSymbols", [](cabacSimpleSequenceDecoder &self, unsigned int numSymbols,
            binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId,
            const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams
        ) {
            
            auto symbols = py::array_t<uint64_t>(numSymbols);

            py::buffer_info buf = symbols.request();
            uint64_t *symbols_ptr = static_cast<uint64_t *>(buf.ptr);

            self.decodeSymbols(symbols_ptr, numSymbols, binId, ctxModelId, binParams, ctxParams);

            return symbols;
        })
        .def("decodeSymbolBypass", [](cabacSimpleSequenceDecoder &self, 
            binarization::BinarizationId binId, const std::vector<unsigned int> binParams
        ) {
            return self.decodeSymbolBypass(binId, binParams);
        })
        .def("decodeSymbol", [](cabacSimpleSequenceDecoder &self, const py::array_t<uint64_t> &symbolsPrev,
            binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId,
            const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams
        ) {
            
            py::buffer_info buf = symbolsPrev.request();
            uint64_t *ptr = static_cast<uint64_t *>(buf.ptr);

            return self.decodeSymbol(ptr, binId, ctxModelId, binParams, ctxParams);
        });

}  // init_pybind_sequence_coding