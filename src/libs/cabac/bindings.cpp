#include <cstdint>
#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>
#include "bin_encoder.h"
#include "bin_decoder.h"
#include "CommonDef.h"
#include <pybind11/functional.h>
#include <pybind11/complex.h>

namespace py = pybind11;

PYBIND11_MODULE(cabac, m) {
    m.doc() = "pybind11 cabac, supporting numpy arrays with reference (no-copy)";

#if RWTH_ENABLE_TRACING
    py::bind_vector<std::vector<std::list<std::pair<uint16_t, uint8_t>>>>(m, "trace");
#endif
    py::class_<cabacEncoder>(m, "cabacEncoder")
        .def(py::init<>())
        .def("start", &cabacEncoder::start)
        .def("finish", &cabacEncoder::finish)
        .def("encodeBinEP", &cabacEncoder::encodeBinEP)
        .def("encodeBinsEP", &cabacEncoder::encodeBinEP)
        .def("encodeBin", &cabacEncoder::encodeBin)

        .def("encodeBins", &cabacEncoder::encodeBins, "Encode a large chunk (np.array) of bins and add it to the bitstream. (Faster than encodeBin, specially for large arrays)")
        
        .def("encodeBinsWrapperNumpy",[](cabacEncoder &self, py::array_t<uint8_t> np_array){
            //Wrapper around encodeBins in order to receive a reference to a numpy array and move the reference as std vector and *avoid* copying data
            py::buffer_info passthroughBuf = np_array.request();
            if (passthroughBuf.ndim != 1) {
                throw std::runtime_error("Error. Number of dimensions must be one");
            }
            size_t length = passthroughBuf.shape[0];
            uint8_t* passthroughPtr = static_cast<uint8_t*>(passthroughBuf.ptr);
            std::vector<uint8_t> passthroughVector(passthroughPtr, passthroughPtr+length);
            self.encodeBins(passthroughVector);
        })


        .def("encodeBinTrm", &cabacEncoder::encodeBinTrm)
        .def("getBitstream", &cabacEncoder::getBitstream)

        .def("getBitstreamWrapperNumpy", [](cabacEncoder &self){
            //Wrapper around getBitstream in order to return a numpy array from the reference of a vector
            auto v = new std::vector<uint8_t>(self.getBitstream());
            auto capsule = py::capsule(v, [](void *v) { delete reinterpret_cast<std::vector<uint8_t>*>(v); });
            return py::array(v->size(), v->data(), capsule);
        })



#if RWTH_ENABLE_TRACING
        .def("getTrace", &cabacEncoder::getTrace)
#endif
        .def("getNumWrittenBits", &cabacEncoder::getNumWrittenBits)
        .def("writeByteAlignment", &cabacEncoder::writeByteAlignment)
        .def("initCtx", &cabacEncoder::initCtx);

    py::class_<cabacDecoder>(m, "cabacDecoder")
        
        //.def(py::init<std::vector<uint8_t>>())
        .def(py::init([](py::array_t<uint8_t> np_array){
            //pass reference to np_array, so that no copy is created in memory
            py::buffer_info passthroughBuf = np_array.request();
            if (passthroughBuf.ndim != 1) {
                throw std::runtime_error("Error. Number of dimensions must be one");
            }
            size_t length = passthroughBuf.shape[0];
            uint8_t* passthroughPtr = static_cast<uint8_t*>(passthroughBuf.ptr);
            std::vector<uint8_t> passthroughVector(passthroughPtr, passthroughPtr+length);

            return new cabacDecoder(passthroughVector);
        }))

        .def("start", &cabacDecoder::start)
        .def("finish", &cabacDecoder::finish)
        .def("decodeBinEP", &cabacDecoder::decodeBinEP)
        .def("decodeBinsEP", &cabacDecoder::decodeBinEP)
        .def("decodeBin", &cabacDecoder::decodeBin)

        .def("decodeBins", &cabacDecoder::decodeBins,"Decode a large chunk (np.array) of bins and add it to the bitstream. (Faster than decodeBin, specially for large arrays)")
        .def("decodeBinsWrapperNumpy", [](cabacDecoder &self,int length){
            //Result will be converted from vector to np array without copying data, just reference
            //The two methods seem to be practically identical in both memory and CPU time (maybe first a little better?)
            
            //1*Try with capsule 
            auto v = new std::vector<uint8_t>(self.decodeBins(length));
            auto capsule = py::capsule(v, [](void *v) { delete reinterpret_cast<std::vector<uint8_t>*>(v); });
            return py::array(v->size(), v->data(), capsule);

            //2*Try with std::move, from ->https://github.com/ssciwr/pybind11-numpy-example/blob/main/python/pybind11-numpy-example_python.cpp
            // std::vector<uint8_t> result_vec = self.decodeBins(length);
            // //TBinDecoder<BinProbModel>::decodeBins(int)
            // // https://github.com/pybind/pybind11/issues/1042
            // // Pass result back to Python.
            // // Ref: https://stackoverflow.com/questions/54876346/pybind11-and-stdvector-how-to-free-data-using-capsules
            // auto* transferToHeapGetRawPtr = new std::vector<uint8_t>(std::move(result_vec));
            // // At this point, transferToHeapGetRawPtr is a raw pointer to an object on the heap. No unique_ptr or shared_ptr, it will have to be freed with delete to avoid a memory leak
            // // Alternate implementation: use a shared_ptr or unique_ptr, but this appears to be more difficult to reason about as a raw pointer (void *) is involved - how does C++ know which destructor to call?
            // const py::capsule freeWhenDone(transferToHeapGetRawPtr, [](void *toFree) {				
            //     delete static_cast<std::vector<uint8_t> *>(toFree);
            // //fmt::print("Free memory."); // Within Python, clear memory to check free: sys.modules[__name__].__dict__.clear()
            // });
            // auto passthroughNumpy = py::array_t<uint8_t>(/*shape=*/{transferToHeapGetRawPtr->size()}, /*strides=*/{sizeof(uint8_t)}, /*ptr=*/transferToHeapGetRawPtr->data(), freeWhenDone);
            // return passthroughNumpy;	
            })


        .def("decodeBinTrm", &cabacDecoder::decodeBinTrm)
        .def("getNumBitsRead", &cabacDecoder::getNumBitsRead)
        .def("initCtx", &cabacDecoder::initCtx);
}
