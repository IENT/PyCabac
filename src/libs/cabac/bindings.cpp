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
    
    
    py::class_<cabacSimpleSequenceEncoder>(m, "cabacSimpleSequenceEncoder")
        .def(py::init<>())
        .def("start", &cabacSimpleSequenceEncoder::start)
        .def("finish", &cabacSimpleSequenceEncoder::finish)
        .def("encodeBinEP", &cabacSimpleSequenceEncoder::encodeBinEP)
        .def("encodeBinsEP", &cabacSimpleSequenceEncoder::encodeBinsEP)
        .def("encodeBin", &cabacSimpleSequenceEncoder::encodeBin)
        .def("encodeBinTrm", &cabacSimpleSequenceEncoder::encodeBinTrm)
        .def("getBitstream", &cabacSimpleSequenceEncoder::getBitstream)
#if RWTH_ENABLE_TRACING
        .def("getTrace", &cabacSimpleSequenceEncoder::getTrace)
#endif
        .def("getNumWrittenBits", &cabacSimpleSequenceEncoder::getNumWrittenBits)
        .def("writeByteAlignment", &cabacSimpleSequenceEncoder::writeByteAlignment)
        .def("initCtx", static_cast<void (cabacSimpleSequenceEncoder::*)(std::vector<std::tuple<double, uint8_t>>)>(&cabacSimpleSequenceEncoder::initCtx), "Initialize contexts with probabilities and shift idxs.")
        .def("initCtx", static_cast<void (cabacSimpleSequenceEncoder::*)(unsigned, double, uint8_t)>(&cabacSimpleSequenceEncoder::initCtx), "Initialize all contexts to same probability and shift idx.")
        .def("encodeBinsBIbypass", &cabacSimpleSequenceEncoder::encodeBinsBIbypass)
        .def("encodeBinsBI", &cabacSimpleSequenceEncoder::encodeBinsBI)
        .def("encodeBinsTUbypass", &cabacSimpleSequenceEncoder::encodeBinsTUbypass, 
            "TU-binarize and and bypass-encode symbol",
            py::arg("input"), py::arg("num_max_bins") = 512
        )
        .def("encodeBinsTU", &cabacSimpleSequenceEncoder::encodeBinsTU,
            "TU-binarize and encode symbol",
            py::arg("input"), py::arg("ctx_ids"), py::arg("num_max_bins") = 512
        )
        .def("encodeBinsEG0bypass", &cabacSimpleSequenceEncoder::encodeBinsEG0bypass)
        .def("encodeBinsEG0", &cabacSimpleSequenceEncoder::encodeBinsEG0);


    py::class_<cabacSimpleSequenceDecoder>(m, "cabacSimpleSequenceDecoder")
        .def(py::init<std::vector<uint8_t>>())
        .def("start", &cabacSimpleSequenceDecoder::start)
        .def("finish", &cabacSimpleSequenceDecoder::finish)
        .def("decodeBinEP", &cabacSimpleSequenceDecoder::decodeBinEP)
        .def("decodeBinsEP", &cabacSimpleSequenceDecoder::decodeBinsEP)
        .def("decodeBin", &cabacSimpleSequenceDecoder::decodeBin)
        .def("decodeBinTrm", &cabacSimpleSequenceDecoder::decodeBinTrm)
        .def("getNumBitsRead", &cabacSimpleSequenceDecoder::getNumBitsRead)
        .def("initCtx", static_cast<void (cabacSimpleSequenceDecoder::*)(std::vector<std::tuple<double, uint8_t>>)>(&cabacSimpleSequenceDecoder::initCtx), "Initialize contexts with probabilities and shift idxs.")
        .def("initCtx", static_cast<void (cabacSimpleSequenceDecoder::*)(unsigned, double, uint8_t)>(&cabacSimpleSequenceDecoder::initCtx), "Initialize all contexts to same probability and shift idx.")
        .def("decodeBinsBIbypass", &cabacSimpleSequenceDecoder::decodeBinsBIbypass)
        .def("decodeBinsBI", &cabacSimpleSequenceDecoder::decodeBinsBI)
        .def("decodeBinsTUbypass", &cabacSimpleSequenceDecoder::decodeBinsTUbypass,
            "bypass-decode TU-encoded symbol", py::arg("num_max_bins") = 512)
        .def("decodeBinsTU", &cabacSimpleSequenceDecoder::decodeBinsTU,
            "decode TU-encoded symbol", py::arg("ctx_ids"), py::arg("num_max_bins") = 512
        )
        .def("decodeBinsEG0bypass", &cabacSimpleSequenceDecoder::decodeBinsEG0bypass)
        .def("decodeBinsEG0", &cabacSimpleSequenceDecoder::decodeBinsEG0);
}
