#include <cstdint>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>

#include "catch/catch.hpp"

#include "cabac/bin_decoder.h"
#include "cabac/bin_encoder.h"
#include "cabac/bitstream.h"


TEST_CASE("test_encodeBinEP")
{
    std::cout << "--- test_encodeBinEP" << std::endl;

    OutputBitstream outputBitstream;
    BinEncoder_Std binEncoder;
    binEncoder.init(&outputBitstream);
    binEncoder.start();
    binEncoder.encodeBinEP(0);
    binEncoder.encodeBinEP(1);
    binEncoder.encodeBinEP(1);
    binEncoder.encodeBinEP(0);
    binEncoder.encodeBinEP(1);

    binEncoder.encodeBinTrm(1);
    binEncoder.finish();
    outputBitstream.writeByteAlignment();

    std::cout << "Byte stream length: " << outputBitstream.getByteStreamLength() << std::endl;

    uint8_t* byteStream = outputBitstream.getByteStream();
    std::vector<uint8_t> byteVector;
    for (int i = 0; i < outputBitstream.getByteStreamLength(); i++) {
        byteVector.push_back(*byteStream);
        byteStream++;
    }

    InputBitstream inputBitstream(byteVector);
    BinDecoder_Std binDecoder;
    binDecoder.init(&inputBitstream);
    binDecoder.start();
    std::cout << "Decoded bin: " << binDecoder.decodeBinEP() << std::endl;
    std::cout << "Decoded bin: " << binDecoder.decodeBinEP() << std::endl;
    std::cout << "Decoded bin: " << binDecoder.decodeBinEP() << std::endl;
    std::cout << "Decoded bin: " << binDecoder.decodeBinEP() << std::endl;
    std::cout << "Decoded bin: " << binDecoder.decodeBinEP() << std::endl;

    binDecoder.finish();
}

TEST_CASE("test_encodeBin")
{
    std::cout << "--- test_encodeBin" << std::endl;

    cabacEncoder binEncoder;
    std::vector<std::tuple<double, uint8_t>> ctxInit  {(0.5, 8)};
    binEncoder.initCtx(ctxInit);
    binEncoder.start();
    binEncoder.encodeBin(0, 0);
    binEncoder.encodeBin(1, 0);
    binEncoder.encodeBinTrm(1);
    binEncoder.finish();
    binEncoder.writeByteAlignment();

    std::vector<uint8_t> byteVector = binEncoder.getBitstream();

    cabacDecoder binDecoder(byteVector);
    binDecoder.initCtx(ctxInit);
    binDecoder.start();
    std::cout << "Decoded bin: " << binDecoder.decodeBin(0) << std::endl;
    std::cout << "Decoded bin: " << binDecoder.decodeBin(0) << std::endl;

    binDecoder.finish();
}
