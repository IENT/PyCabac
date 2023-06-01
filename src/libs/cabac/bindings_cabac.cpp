#include <cstdint>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "bin_encoder.h"
#include "bin_decoder.h"
#include "CommonDef.h"

namespace py = pybind11;

void init_pybind_cabac(py::module &m) {

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
        .def("getNumWrittenBits", &cabacEncoder::getNumWrittenBits)
        .def("writeByteAlignment", &cabacEncoder::writeByteAlignment)
        .def("initCtx", static_cast<void (cabacEncoder::*)(std::vector<std::tuple<double, uint8_t>>)>(&cabacEncoder::initCtx), 
            "Initialize contexts with probabilities and shift idxs."
        )
        .def("initCtx", static_cast<void (cabacEncoder::*)(unsigned, double, uint8_t)>(&cabacEncoder::initCtx),
            "Initialize all contexts to same probability and shift idx."
        );
    
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
        .def("initCtx", static_cast<void (cabacDecoder::*)(std::vector<std::tuple<double, uint8_t>>)>(&cabacDecoder::initCtx),
            "Initialize contexts with probabilities and shift idxs."
        )
        .def("initCtx", static_cast<void (cabacDecoder::*)(unsigned, double, uint8_t)>(&cabacDecoder::initCtx),
            "Initialize all contexts to same probability and shift idx."
        );

    // ---------------------------------------------------------------------------------------------------------------------
    // Encoder with trace enabled
    py::class_<cabacTraceEncoder, cabacEncoder>(m, "cabacTraceEncoder")
        .def(py::init<>())
        .def("encodeBin", &cabacTraceEncoder::encodeBin) // overloaded with tracing enabled
        .def("getTrace", &cabacTraceEncoder::getTrace)
        .def("initCtx", static_cast<void (cabacTraceEncoder::*)(std::vector<std::tuple<double, uint8_t>>)>(&cabacTraceEncoder::initCtx),
            "Initialize contexts with probabilities and shift idxs."
        )
        .def("initCtx", static_cast<void (cabacTraceEncoder::*)(unsigned, double, uint8_t)>(&cabacTraceEncoder::initCtx),
            "Initialize all contexts to same probability and shift idx."
        );

}  // init_pybind_cabac
