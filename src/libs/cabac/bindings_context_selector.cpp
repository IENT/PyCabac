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

namespace py = pybind11;

void init_pybind_context_selector(py::module &m) {
   
    // ---------------------------------------------------------------------------------------------------------------------
    // Context IDs

    // ---------------------------------------------------------------------------------------------------------------------
    // Bin position
    m.def("getContextIdBinPosition", &contextSelector::getContextIdBinPosition, 
        py::arg("n"), py::arg("restPos")=10);
    m.def("getContextIdsBinPosition", [](unsigned int restPos=10, unsigned int numMaxBins=512) {
        std::vector<unsigned int> ctxIDs(numMaxBins, 0);
        contextSelector::getContextIdsBinPosition(ctxIDs,  restPos);
        return ctxIDs;
    });

    // ---------------------------------------------------------------------------------------------------------------------
    // Order 1, bin-to-bin
    // ---------------------------------------------------------------------------------------------------------------------
    m.def("getContextIdBinsOrder1BI", &contextSelector::getContextIdBinsOrder1BI, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("numBins"), py::arg("restPos")=10);
    m.def("getContextIdsBinsOrder1BI", [](uint64_t symbolPrev, unsigned int numBins, unsigned int restPos=10, unsigned int numMaxBins=512) {
        std::vector<unsigned int> ctxIDs(numMaxBins, 0);
        contextSelector::getContextIdsBinsOrder1BI(ctxIDs, symbolPrev, numBins, restPos);
        return ctxIDs;
    });
    m.def("getContextIdBinsOrder1TU", &contextSelector::getContextIdBinsOrder1TU, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("restPos")=10);
    m.def("getContextIdsBinsOrder1TU", [](uint64_t symbolPrev, unsigned int restPos=10, unsigned int numMaxBins=512) {
        std::vector<unsigned int> ctxIDs(numMaxBins, 0);
        contextSelector::getContextIdsBinsOrder1TU(ctxIDs, symbolPrev, restPos);
        return ctxIDs;
    });
    m.def("getContextIdBinsOrder1EGk", &contextSelector::getContextIdBinsOrder1EGk, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("k"), py::arg("restPos")=10);
    m.def("getContextIdsBinsOrder1EGk", [](uint64_t symbolPrev, unsigned int k, unsigned int restPos=10, unsigned int numMaxPrefixBins=24) {
        std::vector<unsigned int> ctxIDs(numMaxPrefixBins, 0);
        contextSelector::getContextIdsBinsOrder1EGk(ctxIDs, symbolPrev, k, restPos);
        return ctxIDs;
    });
    // ---------------------------------------------------------------------------------------------------------------------
    // Order 1, symbol-to-symbol
    // ---------------------------------------------------------------------------------------------------------------------
    m.def("getContextIdSymbolOrder1BI", &contextSelector::getContextIdSymbolOrder1BI, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("restPos")=8, py::arg("symbolMax")=32);
    m.def("getContextIdsSymbolOrder1BI", [](uint64_t symbolPrev, unsigned int restPos=8, unsigned int symbolMax=32, unsigned int numMaxBins=512) {
        std::vector<unsigned int> ctxIDs(numMaxBins, 0);
        contextSelector::getContextIdsSymbolOrder1BI(ctxIDs, symbolPrev, restPos, symbolMax);
        return ctxIDs;
    });
    m.def("getContextIdSymbolOrder1TU", &contextSelector::getContextIdSymbolOrder1TU, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("restPos")=8, py::arg("symbolMax")=32);
    m.def("getContextIdsSymbolOrder1TU", [](uint64_t symbolPrev, unsigned int restPos=8, unsigned int symbolMax=32, unsigned int numMaxBins=512) {
        std::vector<unsigned int> ctxIDs(numMaxBins, 0);
        contextSelector::getContextIdsSymbolOrder1TU(ctxIDs, symbolPrev, restPos, symbolMax);
        return ctxIDs;
    });
    m.def("getContextIdSymbolOrder1EGk", &contextSelector::getContextIdSymbolOrder1EGk, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("k"), py::arg("restPos")=8, py::arg("symbolMax")=32);
    m.def("getContextIdsSymbolOrder1EGk", [](uint64_t symbolPrev, unsigned int k, unsigned int restPos=8, unsigned int symbolMax=32, unsigned int numMaxPrefixBins=24) {
        std::vector<unsigned int> ctxIDs(numMaxPrefixBins, 0);
        contextSelector::getContextIdsSymbolOrder1EGk(ctxIDs, symbolPrev, k, restPos, symbolMax);
        return ctxIDs;
    });

    // ---------------------------------------------------------------------------------------------------------------------
    // Generalized functions
    // Use these for order > 1
    // ---------------------------------------------------------------------------------------------------------------------
    m.def("getContextId", [](const unsigned int n, const unsigned int d, const py::array_t<uint64_t> &symbolsPrev,
        binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId,
        const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams, unsigned int numMaxCtxs = 24
    ) {
        auto buf = symbolsPrev.request();
        uint64_t *ptr = static_cast<uint64_t *>(buf.ptr);
        return contextSelector::getContextId(n, d, ptr, binId, ctxModelId, binParams, ctxParams);
    });
    // ---------------------------------------------------------------------------------------------------------------------
    m.def("getContextIds", [](const unsigned int d, const py::array_t<uint64_t> &symbolsPrev,
        binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId,
        const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams, unsigned int numMaxCtxs = 24
    ) {
        auto buf = symbolsPrev.request();
        uint64_t *ptr = static_cast<uint64_t *>(buf.ptr);
        std::vector<unsigned int> ctxIDs(numMaxCtxs, 0);
        contextSelector::getContextIds(ctxIDs, d, ptr, binId, ctxModelId, binParams, ctxParams);
        return ctxIDs;
    });
    // ---------------------------------------------------------------------------------------------------------------------
    m.def("getNumContexts", [](binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId,
        const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams) {
        return contextSelector::getNumContexts(binId, ctxModelId, binParams, ctxParams);
    });
    // ---------------------------------------------------------------------------------------------------------------------

    py::enum_<binarization::BinarizationId>(m, "BinarizationId")
        .value("BI", binarization::BinarizationId::BI)
        .value("TU", binarization::BinarizationId::TU)
        .value("EGk", binarization::BinarizationId::EGk)
        .value("NA", binarization::BinarizationId::NA)
        .value("RICE", binarization::BinarizationId::RICE);

    py::enum_<contextSelector::ContextModelId>(m, "ContextModelId")
        .value("BAC", contextSelector::ContextModelId::BAC)
        .value("BINPOSITION", contextSelector::ContextModelId::BINPOSITION)
        .value("BINSORDERN", contextSelector::ContextModelId::BINSORDERN)
        .value("SYMBOLORDERN", contextSelector::ContextModelId::SYMBOLORDERN)
        .value("SYMBOLPOSITION", contextSelector::ContextModelId::SYMBOLPOSITION)
        .value("BINSYMBOLPOSITION", contextSelector::ContextModelId::BINSYMBOLPOSITION)
        .value("BINSORDERNSYMBOLPOSITION", contextSelector::ContextModelId::BINSORDERNSYMBOLPOSITION)
        .value("SYMBOLORDERNSYMBOLPOSITION", contextSelector::ContextModelId::SYMBOLORDERNSYMBOLPOSITION);

}  // init_pybind_context_selector