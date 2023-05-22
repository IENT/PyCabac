#include <cstdint>
#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>
#include <vector>
#include "bin_encoder.h"
#include "bin_decoder.h"
#include "CommonDef.h"

namespace py = pybind11;

PYBIND11_MODULE(cabac, m) {
    m.doc() = "pybind11 eabac";

#if RWTH_ENABLE_TRACING
    py::bind_vector<std::vector<std::list<std::pair<uint16_t, uint8_t>>>>(m, "trace");
#endif
    // ---------------------------------------------------------------------------------------------------------------------
    // Encoder
    py::class_<cabacEncoder>(m, "cabacEncoder")
        .def(py::init<>())
        .def("start", &cabacEncoder::start)
        .def("finish", &cabacEncoder::finish)
        .def("encodeBinEP", &cabacEncoder::encodeBinEP)
        .def("encodeBinsEP", &cabacEncoder::encodeBinsEP)
        .def("encodeBin", &cabacEncoder::encodeBin)
        .def("encodeBinTrm", &cabacEncoder::encodeBinTrm)
        .def("getBitstream", &cabacEncoder::getBitstream)
#if RWTH_ENABLE_TRACING
        .def("getTrace", &cabacEncoder::getTrace)
#endif
        .def("getNumWrittenBits", &cabacEncoder::getNumWrittenBits)
        .def("writeByteAlignment", &cabacEncoder::writeByteAlignment)
        .def("initCtx", static_cast<void (cabacEncoder::*)(std::vector<std::tuple<double, uint8_t>>)>(&cabacEncoder::initCtx), "Initialize contexts with probabilities and shift idxs.")
        .def("initCtx", static_cast<void (cabacEncoder::*)(unsigned, double, uint8_t)>(&cabacEncoder::initCtx), "Initialize all contexts to same probability and shift idx.");
    
    // ---------------------------------------------------------------------------------------------------------------------
    // Decoder
    py::class_<cabacDecoder>(m, "cabacDecoder")
        .def(py::init<std::vector<uint8_t>>())
        .def("start", &cabacDecoder::start)
        .def("finish", &cabacDecoder::finish)
        .def("decodeBinEP", &cabacDecoder::decodeBinEP)
        .def("decodeBinsEP", &cabacDecoder::decodeBinsEP)
        .def("decodeBin", &cabacDecoder::decodeBin)
        .def("decodeBinTrm", &cabacDecoder::decodeBinTrm)
        .def("getNumBitsRead", &cabacDecoder::getNumBitsRead)
        .def("initCtx", static_cast<void (cabacDecoder::*)(std::vector<std::tuple<double, uint8_t>>)>(&cabacDecoder::initCtx), "Initialize contexts with probabilities and shift idxs.")
        .def("initCtx", static_cast<void (cabacDecoder::*)(unsigned, double, uint8_t)>(&cabacDecoder::initCtx), "Initialize all contexts to same probability and shift idx.");

    // ---------------------------------------------------------------------------------------------------------------------    
    // SymbolEncoder
    py::class_<cabacSymbolEncoder, cabacEncoder>(m, "cabacSymbolEncoder")
        .def(py::init<>())
        .def("encodeBinsBIbypass", &cabacSymbolEncoder::encodeBinsBIbypass)
        .def("encodeBinsBI", &cabacSymbolEncoder::encodeBinsBI)
        .def("encodeBinsTUbypass", &cabacSymbolEncoder::encodeBinsTUbypass, 
            "TU-binarize and and bypass-encode symbol",
            py::arg("symbol"), py::arg("numMaxBins") = 512
        )
        .def("encodeBinsTU", &cabacSymbolEncoder::encodeBinsTU,
            "TU-binarize and encode symbol",
            py::arg("symbol"), py::arg("ctxIds"), py::arg("numMaxBins") = 512
        )
        .def("encodeBinsEG0bypass", &cabacSymbolEncoder::encodeBinsEG0bypass)
        .def("encodeBinsEG0", &cabacSymbolEncoder::encodeBinsEG0)
        .def("encodeBinsEGkbypass", &cabacSymbolEncoder::encodeBinsEGkbypass)
        .def("encodeBinsEGk", &cabacSymbolEncoder::encodeBinsEGk);

    // ---------------------------------------------------------------------------------------------------------------------
    // SymbolDecoder
    py::class_<cabacSymbolDecoder, cabacDecoder>(m, "cabacSymbolDecoder")
        .def(py::init<std::vector<uint8_t>>())
        .def("decodeBinsBIbypass", &cabacSymbolDecoder::decodeBinsBIbypass)
        .def("decodeBinsBI", &cabacSymbolDecoder::decodeBinsBI)
        .def("decodeBinsTUbypass", &cabacSymbolDecoder::decodeBinsTUbypass,
            "bypass-decode TU-encoded symbol", py::arg("numMaxBins") = 512)
        .def("decodeBinsTU", &cabacSymbolDecoder::decodeBinsTU,
            "decode TU-encoded symbol", py::arg("ctxIds"), py::arg("numMaxBins") = 512
        )
        .def("decodeBinsEG0bypass", &cabacSymbolDecoder::decodeBinsEG0bypass)
        .def("decodeBinsEG0", &cabacSymbolDecoder::decodeBinsEG0)
        .def("decodeBinsEGkbypass", &cabacSymbolDecoder::decodeBinsEGkbypass)
        .def("decodeBinsEGk", &cabacSymbolDecoder::decodeBinsEGk);

    // ---------------------------------------------------------------------------------------------------------------------
    // SequenceEncoder
    py::class_<cabacSimpleSequenceEncoder, cabacSymbolEncoder>(m, "cabacSimpleSequenceEncoder")
        .def(py::init<>())
        .def("encodeBinsTUorder1", &cabacSimpleSequenceEncoder::encodeBinsTUorder1,
            "TU-binarize and encode symbol with simple order1-context model",
            py::arg("symbol"), py::arg("symbolPrev"), py::arg("restPos") = 10, py::arg("numMaxBins") = 512
        )
        .def("encodeBinsEG0order1", &cabacSimpleSequenceEncoder::encodeBinsEG0order1,
            "EG0-binarize and encode symbol with simple order1-context model",
            py::arg("symbol"), py::arg("symbolPrev"), py::arg("restPos") = 10, py::arg("numMaxPrefixBins") = 24
        )
        .def("encodeBinsEGkorder1", &cabacSimpleSequenceEncoder::encodeBinsEGkorder1,
            "EGk-binarize and encode symbol with simple order1-context model",
            py::arg("symbol"), py::arg("symbolPrev"), py::arg("k"), py::arg("restPos") = 10, py::arg("numMaxPrefixBins") = 24
        );

    // ---------------------------------------------------------------------------------------------------------------------
    // SequenceDecoder
    py::class_<cabacSimpleSequenceDecoder, cabacSymbolDecoder>(m, "cabacSimpleSequenceDecoder")
        .def(py::init<std::vector<uint8_t>>())
        .def("decodeBinsTUorder1", &cabacSimpleSequenceDecoder::decodeBinsTUorder1,
            "decode TU-binarized symbol with simple order1-context model",
            py::arg("symbolPrev"), py::arg("restPos") = 10, py::arg("numMaxBins") = 512
        )
        .def("decodeBinsEG0order1", &cabacSimpleSequenceDecoder::decodeBinsEG0order1,
            "decode EG0-binarized symbol with simple order1-context model",
            py::arg("symbolPrev"), py::arg("restPos") = 10, py::arg("numMaxPrefixBins") = 24
        )
        .def("decodeBinsEGkorder1", &cabacSimpleSequenceDecoder::decodeBinsEGkorder1,
            "decode EGk-binarized symbol with simple order1-context model",
            py::arg("symbolPrev"), py::arg("k"), py::arg("restPos") = 10, py::arg("numMaxPrefixBins") = 24
        );

    // ---------------------------------------------------------------------------------------------------------------------
    // Context IDs
    m.def("getContextIdBinsOrder1TU", &contextSelector::getContextIdBinsOrder1TU, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("restPos") = 10);
    m.def("getContextIdsBinsOrder1TU", [](unsigned int symbolPrev, unsigned int restPos = 10, unsigned int numMaxBins = 512) {
        std::vector<unsigned int> ctxIDs(numMaxBins, 0);
        contextSelector::getContextIdsBinsOrder1TU(ctxIDs, symbolPrev, restPos);
        return ctxIDs;
    });
    m.def("getContextIdBinsOrder1EG0", &contextSelector::getContextIdBinsOrder1EG0, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("restPos") = 10);
    m.def("getContextIdsBinsOrder1EG0", [](unsigned int symbolPrev, unsigned int restPos = 10, unsigned int numMaxPrefixBins = 24) {
        std::vector<unsigned int> ctxIDs(numMaxPrefixBins, 0);
        contextSelector::getContextIdsBinsOrder1EG0(ctxIDs, symbolPrev, restPos);
        return ctxIDs;
    });
    m.def("getContextIdBinsOrder1EGk", &contextSelector::getContextIdBinsOrder1EGk, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("k"), py::arg("restPos") = 10);
    m.def("getContextIdsBinsOrder1EGk", [](unsigned int symbolPrev, unsigned int k, unsigned int restPos = 10, unsigned int numMaxPrefixBins = 24) {
        std::vector<unsigned int> ctxIDs(numMaxPrefixBins, 0);
        contextSelector::getContextIdsBinsOrder1EGk(ctxIDs, symbolPrev, k, restPos);
        return ctxIDs;
    });

    py::enum_<contextSelector::ContextModelId>(m, "ContextModelId")
        .value("BINSORDER1", contextSelector::ContextModelId::BINSORDER1)
        .value("SYMBOLORDER1", contextSelector::ContextModelId::SYMBOLORDER1);

}  // PYBIND11_MODULE(cabac, m)
