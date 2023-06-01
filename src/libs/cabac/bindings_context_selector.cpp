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
    m.def("getContextIdBinsOrder1EG0", &contextSelector::getContextIdBinsOrder1EG0, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("restPos")=10);
    m.def("getContextIdsBinsOrder1EG0", [](uint64_t symbolPrev, unsigned int restPos=10, unsigned int numMaxPrefixBins=24) {
        std::vector<unsigned int> ctxIDs(numMaxPrefixBins, 0);
        contextSelector::getContextIdsBinsOrder1EG0(ctxIDs, symbolPrev, restPos);
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
    m.def("getContextIdSymbolOrder1EG0", &contextSelector::getContextIdSymbolOrder1EG0, 
        py::arg("n"), py::arg("symbolPrev"), py::arg("restPos")=8, py::arg("symbolMax")=32);
    m.def("getContextIdsSymbolOrder1EG0", [](uint64_t symbolPrev, unsigned int restPos=8, unsigned int symbolMax=32, unsigned int numMaxPrefixBins=24) {
        std::vector<unsigned int> ctxIDs(numMaxPrefixBins, 0);
        contextSelector::getContextIdsSymbolOrder1EG0(ctxIDs, symbolPrev, restPos, symbolMax);
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
    m.def("getContextIds", [](uint64_t symbolPrev,
        binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId,
        const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams, unsigned int numMaxCtxs = 24
    ) {
        std::vector<unsigned int> ctxIDs(numMaxCtxs, 0);
        contextSelector::getContextIds(ctxIDs, symbolPrev, binId, ctxModelId, binParams, ctxParams);
        return ctxIDs;
    });
    // ---------------------------------------------------------------------------------------------------------------------
    py::enum_<binarization::BinarizationId>(m, "BinarizationId")
        .value("BI", binarization::BinarizationId::BI)
        .value("TU", binarization::BinarizationId::TU)
        .value("EG0", binarization::BinarizationId::EG0)
        .value("EGk", binarization::BinarizationId::EGk);

    py::enum_<contextSelector::ContextModelId>(m, "ContextModelId")
        .value("BINSORDER1", contextSelector::ContextModelId::BINSORDER1)
        .value("SYMBOLORDER1", contextSelector::ContextModelId::SYMBOLORDER1);

}  // init_pybind_context_selector