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

PYBIND11_MODULE(cabac, m) {
    m.doc() = "pybind11 eabac";

    // See comment above
    // py::bind_vector<std::vector<unsigned int>>(m, "VectorUnsignedInt", py::buffer_protocol());
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
        .def("encodeBin", &cabacTraceEncoder::encodeBin)
        .def("getTrace", &cabacTraceEncoder::getTrace)
        .def("initCtx", static_cast<void (cabacTraceEncoder::*)(std::vector<std::tuple<double, uint8_t>>)>(&cabacTraceEncoder::initCtx),
            "Initialize contexts with probabilities and shift idxs."
        )
        .def("initCtx", static_cast<void (cabacTraceEncoder::*)(unsigned, double, uint8_t)>(&cabacTraceEncoder::initCtx),
            "Initialize all contexts to same probability and shift idx."
        );

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
            py::arg("symbol"), py::arg("numMaxBins") = 512
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
            "bypass-decode TU-encoded symbol", py::arg("numMaxBins") = 512)
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

    // ---------------------------------------------------------------------------------------------------------------------
    // SequenceEncoder
    py::class_<cabacSimpleSequenceEncoder, cabacSymbolEncoder>(m, "cabacSimpleSequenceEncoder")
        .def(py::init<>())
        .def("encodeBinsTUbinsOrder1", &cabacSimpleSequenceEncoder::encodeBinsTUbinsOrder1,
            "TU-binarize and encode symbol with simple order1-context model",
            py::arg("symbol"), py::arg("symbolPrev"), py::arg("restPos") = 10, py::arg("numMaxBins") = 512
        )
        .def("encodeBinsEG0binsOrder1", &cabacSimpleSequenceEncoder::encodeBinsEG0binsOrder1,
            "EG0-binarize and encode symbol with simple order1-context model",
            py::arg("symbol"), py::arg("symbolPrev"), py::arg("restPos") = 10, py::arg("numMaxPrefixBins") = 24
        )
        .def("encodeBinsEGkbinsOrder1", &cabacSimpleSequenceEncoder::encodeBinsEGkbinsOrder1,
            "EGk-binarize and encode symbol with simple order1-context model",
            py::arg("symbol"), py::arg("symbolPrev"), py::arg("k"), py::arg("restPos") = 10, py::arg("numMaxPrefixBins") = 24
        )
        .def("encodeBinsTUsymbolOrder1", &cabacSimpleSequenceEncoder::encodeBinsTUsymbolOrder1,
            "TU-binarize and encode symbol with simple order1-context model",
            py::arg("symbol"), py::arg("symbolPrev"), py::arg("restPos") = 8, py::arg("symbolMax")=32, py::arg("numMaxBins") = 512
        )
        .def("encodeBinsEG0symbolOrder1", &cabacSimpleSequenceEncoder::encodeBinsEG0symbolOrder1,
            "EG0-binarize and encode symbol with simple order1-context model",
            py::arg("symbol"), py::arg("symbolPrev"), py::arg("restPos") = 8, py::arg("symbolMax")=32, py::arg("numMaxPrefixBins") = 24
        )
        .def("encodeBinsEGksymbolOrder1", &cabacSimpleSequenceEncoder::encodeBinsEGksymbolOrder1,
            "EGk-binarize and encode symbol with simple order1-context model",
            py::arg("symbol"), py::arg("symbolPrev"), py::arg("k"), py::arg("restPos") = 8, py::arg("symbolMax")=32, py::arg("numMaxPrefixBins") = 24
        )
        .def("encodeSymbols", &cabacSimpleSequenceEncoder::encodeSymbols)
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
            py::arg("symbolPrev"), py::arg("restPos") = 10, py::arg("numMaxBins") = 512
        )
        .def("decodeBinsEG0binsOrder1", &cabacSimpleSequenceDecoder::decodeBinsEG0binsOrder1,
            "decode EG0-binarized symbol with simple order1-context model",
            py::arg("symbolPrev"), py::arg("restPos") = 10, py::arg("numMaxPrefixBins") = 24
        )
        .def("decodeBinsEGkbinsOrder1", &cabacSimpleSequenceDecoder::decodeBinsEGkbinsOrder1,
            "decode EGk-binarized symbol with simple order1-context model",
            py::arg("symbolPrev"), py::arg("k"), py::arg("restPos") = 10, py::arg("numMaxPrefixBins") = 24
        )
        .def("decodeBinsTUsymbolOrder1", &cabacSimpleSequenceDecoder::decodeBinsTUsymbolOrder1,
            "decode TU-binarized symbol with simple order1-context model",
            py::arg("symbolPrev"), py::arg("restPos") = 8, py::arg("symbolMax")=32, py::arg("numMaxBins") = 512
        )
        .def("decodeBinsEG0symbolOrder1", &cabacSimpleSequenceDecoder::decodeBinsEG0symbolOrder1,
            "decode EG0-binarized symbol with simple order1-context model",
            py::arg("symbolPrev"), py::arg("restPos") = 8, py::arg("symbolMax")=32, py::arg("numMaxPrefixBins") = 24
        )
        .def("decodeBinsEGksymbolOrder1", &cabacSimpleSequenceDecoder::decodeBinsEGksymbolOrder1,
            "decode EGk-binarized symbol with simple order1-context model",
            py::arg("symbolPrev"), py::arg("k"), py::arg("restPos") = 8, py::arg("symbolMax")=32, py::arg("numMaxPrefixBins") = 24
        )
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

    // ---------------------------------------------------------------------------------------------------------------------
    // Context IDs
    m.def("getContextIdBinsOrder1BI", &contextSelector::getContextIdBinsOrder1BI, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("numBins"), py::arg("restPos") = 10);
    m.def("getContextIdsBinsOrder1BI", [](uint64_t symbolPrev, unsigned int numBins, unsigned int restPos = 10, unsigned int numMaxBins = 512) {
        std::vector<unsigned int> ctxIDs(numMaxBins, 0);
        contextSelector::getContextIdsBinsOrder1BI(ctxIDs, symbolPrev, numBins, restPos);
        return ctxIDs;
    });
    m.def("getContextIdBinsOrder1TU", &contextSelector::getContextIdBinsOrder1TU, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("restPos") = 10);
    m.def("getContextIdsBinsOrder1TU", [](uint64_t symbolPrev, unsigned int restPos = 10, unsigned int numMaxBins = 512) {
        std::vector<unsigned int> ctxIDs(numMaxBins, 0);
        contextSelector::getContextIdsBinsOrder1TU(ctxIDs, symbolPrev, restPos);
        return ctxIDs;
    });
    m.def("getContextIdBinsOrder1EG0", &contextSelector::getContextIdBinsOrder1EG0, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("restPos") = 10);
    m.def("getContextIdsBinsOrder1EG0", [](uint64_t symbolPrev, unsigned int restPos = 10, unsigned int numMaxPrefixBins = 24) {
        std::vector<unsigned int> ctxIDs(numMaxPrefixBins, 0);
        contextSelector::getContextIdsBinsOrder1EG0(ctxIDs, symbolPrev, restPos);
        return ctxIDs;
    });
    m.def("getContextIdBinsOrder1EGk", &contextSelector::getContextIdBinsOrder1EGk, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("k"), py::arg("restPos") = 10);
    m.def("getContextIdsBinsOrder1EGk", [](uint64_t symbolPrev, unsigned int k, unsigned int restPos = 10, unsigned int numMaxPrefixBins = 24) {
        std::vector<unsigned int> ctxIDs(numMaxPrefixBins, 0);
        contextSelector::getContextIdsBinsOrder1EGk(ctxIDs, symbolPrev, k, restPos);
        return ctxIDs;
    });
    m.def("getContextIdSymbolOrder1BI", &contextSelector::getContextIdSymbolOrder1BI, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("restPos") = 8, py::arg("symbolMax")=32);
    m.def("getContextIdsSymbolOrder1BI", [](uint64_t symbolPrev, unsigned int restPos = 8, unsigned int symbolMax=32, unsigned int numMaxBins = 512) {
        std::vector<unsigned int> ctxIDs(numMaxBins, 0);
        contextSelector::getContextIdsSymbolOrder1BI(ctxIDs, symbolPrev, restPos, symbolMax);
        return ctxIDs;
    });
    m.def("getContextIdSymbolOrder1TU", &contextSelector::getContextIdSymbolOrder1TU, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("restPos") = 8, py::arg("symbolMax")=32);
    m.def("getContextIdsSymbolOrder1TU", [](uint64_t symbolPrev, unsigned int restPos = 8, unsigned int symbolMax=32, unsigned int numMaxBins = 512) {
        std::vector<unsigned int> ctxIDs(numMaxBins, 0);
        contextSelector::getContextIdsSymbolOrder1TU(ctxIDs, symbolPrev, restPos, symbolMax);
        return ctxIDs;
    });
    m.def("getContextIdSymbolOrder1EG0", &contextSelector::getContextIdSymbolOrder1EG0, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("restPos") = 8, py::arg("symbolMax")=32);
    m.def("getContextIdsSymbolOrder1EG0", [](uint64_t symbolPrev, unsigned int restPos = 8, unsigned int symbolMax=32, unsigned int numMaxPrefixBins = 24) {
        std::vector<unsigned int> ctxIDs(numMaxPrefixBins, 0);
        contextSelector::getContextIdsSymbolOrder1EG0(ctxIDs, symbolPrev, restPos, symbolMax);
        return ctxIDs;
    });
    m.def("getContextIdSymbolOrder1EGk", &contextSelector::getContextIdSymbolOrder1EGk, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("k"), py::arg("restPos") = 8, py::arg("symbolMax")=32);
    m.def("getContextIdsSymbolOrder1EGk", [](uint64_t symbolPrev, unsigned int k, unsigned int restPos = 8, unsigned int symbolMax=32, unsigned int numMaxPrefixBins = 24) {
        std::vector<unsigned int> ctxIDs(numMaxPrefixBins, 0);
        contextSelector::getContextIdsSymbolOrder1EGk(ctxIDs, symbolPrev, k, restPos, symbolMax);
        return ctxIDs;
    });
    m.def("getContextIds", [](uint64_t symbolPrev,
        binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId,
        const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams, unsigned int numMaxCtxs = 24
    ) {
        std::vector<unsigned int> ctxIDs(numMaxCtxs, 0);
        contextSelector::getContextIds(ctxIDs, symbolPrev, binId, ctxModelId, binParams, ctxParams);
        return ctxIDs;
    });

    py::enum_<binarization::BinarizationId>(m, "BinarizationId")
        .value("BI", binarization::BinarizationId::BI)
        .value("TU", binarization::BinarizationId::TU)
        .value("EG0", binarization::BinarizationId::EG0)
        .value("EGk", binarization::BinarizationId::EGk);

    py::enum_<contextSelector::ContextModelId>(m, "ContextModelId")
        .value("BINSORDER1", contextSelector::ContextModelId::BINSORDER1)
        .value("SYMBOLORDER1", contextSelector::ContextModelId::SYMBOLORDER1);

}  // PYBIND11_MODULE(cabac, m)
