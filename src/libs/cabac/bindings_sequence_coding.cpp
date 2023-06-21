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
        .def("encodeBinsTUbinsOrder1", &cabacSimpleSequenceEncoder::encodeBinsTUbinsOrder1,
            "TU-binarize and encode symbol with simple order1-context model",
            py::arg("symbol"), py::arg("symbolPrev"), py::arg("restPos")=10, py::arg("numMaxBins")=512
        )
        .def("encodeBinsEG0binsOrder1", &cabacSimpleSequenceEncoder::encodeBinsEG0binsOrder1,
            "EG0-binarize and encode symbol with simple order1-context model",
            py::arg("symbol"), py::arg("symbolPrev"), py::arg("restPos")=10, py::arg("numMaxPrefixBins")=24
        )
        .def("encodeBinsEGkbinsOrder1", &cabacSimpleSequenceEncoder::encodeBinsEGkbinsOrder1,
            "EGk-binarize and encode symbol with simple order1-context model",
            py::arg("symbol"), py::arg("symbolPrev"), py::arg("k"), py::arg("restPos")=10, py::arg("numMaxPrefixBins")=24
        )
        .def("encodeBinsTUsymbolOrder1", &cabacSimpleSequenceEncoder::encodeBinsTUsymbolOrder1,
            "TU-binarize and encode symbol with simple order1-context model",
            py::arg("symbol"), py::arg("symbolPrev"), py::arg("restPos")=8, py::arg("symbolMax")=32, py::arg("numMaxBins")=512
        )
        .def("encodeBinsEG0symbolOrder1", &cabacSimpleSequenceEncoder::encodeBinsEG0symbolOrder1,
            "EG0-binarize and encode symbol with simple order1-context model",
            py::arg("symbol"), py::arg("symbolPrev"), py::arg("restPos")=8, py::arg("symbolMax")=32, py::arg("numMaxPrefixBins")=24
        )
        .def("encodeBinsEGksymbolOrder1", &cabacSimpleSequenceEncoder::encodeBinsEGksymbolOrder1,
            "EGk-binarize and encode symbol with simple order1-context model",
            py::arg("symbol"), py::arg("symbolPrev"), py::arg("k"), py::arg("restPos")=8, py::arg("symbolMax")=32, py::arg("numMaxPrefixBins")=24
        )
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
        });


    // ---------------------------------------------------------------------------------------------------------------------
    // SequenceDecoder
    py::class_<cabacSimpleSequenceDecoder, cabacSymbolDecoder>(m, "cabacSimpleSequenceDecoder")
        .def(py::init<std::vector<uint8_t>>())
        .def("decodeBinsTUbinsOrder1", &cabacSimpleSequenceDecoder::decodeBinsTUbinsOrder1,
            "decode TU-binarized symbol with simple order1-context model",
            py::arg("symbolPrev"), py::arg("restPos")=10, py::arg("numMaxBins")=512
        )
        .def("decodeBinsEG0binsOrder1", &cabacSimpleSequenceDecoder::decodeBinsEG0binsOrder1,
            "decode EG0-binarized symbol with simple order1-context model",
            py::arg("symbolPrev"), py::arg("restPos")=10, py::arg("numMaxPrefixBins")=24
        )
        .def("decodeBinsEGkbinsOrder1", &cabacSimpleSequenceDecoder::decodeBinsEGkbinsOrder1,
            "decode EGk-binarized symbol with simple order1-context model",
            py::arg("symbolPrev"), py::arg("k"), py::arg("restPos")=10, py::arg("numMaxPrefixBins")=24
        )
        .def("decodeBinsTUsymbolOrder1", &cabacSimpleSequenceDecoder::decodeBinsTUsymbolOrder1,
            "decode TU-binarized symbol with simple order1-context model",
            py::arg("symbolPrev"), py::arg("restPos")=8, py::arg("symbolMax")=32, py::arg("numMaxBins")=512
        )
        .def("decodeBinsEG0symbolOrder1", &cabacSimpleSequenceDecoder::decodeBinsEG0symbolOrder1,
            "decode EG0-binarized symbol with simple order1-context model",
            py::arg("symbolPrev"), py::arg("restPos")=8, py::arg("symbolMax")=32, py::arg("numMaxPrefixBins")=24
        )
        .def("decodeBinsEGksymbolOrder1", &cabacSimpleSequenceDecoder::decodeBinsEGksymbolOrder1,
            "decode EGk-binarized symbol with simple order1-context model",
            py::arg("symbolPrev"), py::arg("k"), py::arg("restPos")=8, py::arg("symbolMax")=32, py::arg("numMaxPrefixBins")=24
        )
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
        });

}  // init_pybind_sequence_coding