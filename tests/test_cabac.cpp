#include <cstdint>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>
#include <random>

#include "catch/catch.hpp"

#include "cabac/bin_decoder.h"
#include "cabac/bin_encoder.h"
#include "cabac/bitstream.h"
#include "common.h"


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
    std::vector<std::tuple<double, uint8_t>> ctxInit {{0.5, 8}};
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


TEST_CASE("test_encodeStepFunction")
{
    std::cout << "--- test_encodeBin" << std::endl;

    cabacEncoder binEncoder;
    std::vector<std::tuple<double, uint8_t>> ctxInit {{0.5, 8}};
    binEncoder.initCtx(ctxInit);

    binEncoder.start();
    // Encode unit step function
    std::vector<unsigned int> symbols = std::vector<unsigned int>(2000, 0);
    for (unsigned int i = 1000; i < 2000; i++){
        symbols[i] = 1;
    }

    for (unsigned int i = 0; i < symbols.size(); i++) {
        binEncoder.encodeBin(symbols[i], 0); // 1027
    }
    binEncoder.encodeBinTrm(1);
    binEncoder.finish();
    binEncoder.writeByteAlignment();

    std::vector<uint8_t> byteVector = binEncoder.getBitstream();

    cabacDecoder binDecoder(byteVector);
    binDecoder.initCtx(ctxInit);
    binDecoder.start();

    std::vector<unsigned int> symbolsDecoded = std::vector<unsigned int>(symbols.size(), 0);

    for(unsigned int i=0; i<2000; i++){
        symbolsDecoded[i] = binDecoder.decodeBin(0);
    }

    binDecoder.decodeBinTrm();
    binDecoder.finish();

    REQUIRE_THAT(symbols, Catch::Matchers::UnorderedEquals(symbolsDecoded));
}

TEST_CASE("test_encodeSymbols")
{
    const int maxVal = 255;
    const int numSymbols = 1000;
    const int k = 1;
    std::cout << "--- test_encodeSymbols" << std::endl;

    
    std::srand(unsigned(std::time(nullptr)));
    std::vector<int> symbols(numSymbols);
    std::generate(symbols.begin(), symbols.end(), []() {
        return rand() % maxVal;
    });
    
    //std::vector<unsigned> symbols = {0, 1, 2, 3, 4, 5, 6, 7};
    

    cabacSimpleSequenceEncoder binEncoder;

    binEncoder.initCtx(1, 0.5, 8);
    binEncoder.start();
    std::vector<unsigned> ctx_ids(512, 0);
    //binEncoder.encodeBinsEG0bypass(5);
    for (unsigned int i = 0; i < symbols.size(); i++){
        //binEncoder.encodeBinsEGk(symbols[i], k, ctx_ids);
        binEncoder.encodeBinsTU(symbols[i], ctx_ids);
    }
    
    binEncoder.encodeBinTrm(1);
    binEncoder.finish();
    binEncoder.writeByteAlignment();

    std::vector<uint8_t> byteVector = binEncoder.getBitstream();

    cabacSimpleSequenceDecoder binDecoder(byteVector);
    binDecoder.initCtx(1, 0.5, 8);
    binDecoder.start();
    std::vector<unsigned int> symbolsDecoded = std::vector<unsigned int>(symbols.size(), 0);
    for (unsigned int i = 0; i < symbolsDecoded.size(); i++){
        symbolsDecoded[i] = binDecoder.decodeBinsTU(ctx_ids);
        // std::cout << "Decoded symbol: " << symbolsDecoded[i] << std::endl;
    }
    binDecoder.decodeBinTrm();
    binDecoder.finish();

    //REQUIRE_THAT(symbols, Catch::Matchers::UnorderedEquals(symbolsDecoded));
}

TEST_CASE("test_encodeSymbolsBinsOrder1")
{
    const int maxVal = 255;
    const int numSymbols = 1000;
    const int restPos = 10;
    const int num_ctx = restPos * 3 + 1;
    const int numMaxPrefixBins = 12;
    const int numBins = 8;
    std::cout << "--- test_encodeSymbolsBinsOrder1" << std::endl;

    std::vector<uint64_t> symbols(numSymbols);
    fillVectorRandomGeometric(&symbols);
    
    //std::vector<uint64_t> symbols = {0, 1, 2, 3, 4, 5, 6, 7};

    cabacSimpleSequenceEncoder binEncoder;
    binEncoder.initCtx(num_ctx, 0.5, 8);
    binEncoder.start();

    uint64_t symbolPrev = 0;
    for (unsigned int i = 0; i < symbols.size(); i++) {
        if(i > 0){
            symbolPrev = symbols[i-1];
        }
        binEncoder.encodeBinsEG0binsOrder1(symbols[i], symbolPrev, restPos, numMaxPrefixBins);
        //binEncoder.encodeBinsTUbinsOrder1(symbols[i], symbolPrev, restPos, maxVal);
    }

    binEncoder.encodeBinTrm(1);
    binEncoder.finish();
    binEncoder.writeByteAlignment();

    std::vector<uint8_t> byteVector = binEncoder.getBitstream();

    cabacSimpleSequenceDecoder binDecoder(byteVector);
    binDecoder.initCtx(num_ctx, 0.5, 8);
    binDecoder.start();
    std::vector<uint64_t> symbolsDecoded = std::vector<uint64_t>(symbols.size(), 0);
    uint64_t symbolDecodedPrev = 0;
    for (unsigned int i = 0; i < symbolsDecoded.size(); i++) {
        if(i > 0){
            symbolDecodedPrev = symbolsDecoded[i-1];
        }
        symbolsDecoded[i] = binDecoder.decodeBinsEG0binsOrder1(symbolDecodedPrev, restPos, numMaxPrefixBins);
        //symbolsDecoded[i] = binDecoder.decodeBinsTUbinsOrder1(symbolDecodedPrev, restPos, maxVal);
        //std::cout << "Decoded symbol: " << symbolsDecoded[i] << std::endl;
    }
    binDecoder.decodeBinTrm();
    binDecoder.finish();
    
    REQUIRE_THAT(symbols, Catch::Matchers::UnorderedEquals(symbolsDecoded));
}

TEST_CASE("test_encodeSymbolsSymbolsOrder1_TU")
{
    const int maxVal = 5000;
    const int numSymbols = 1000;
    const int restPos = 10;
    const int ctxSymbolMax = 16;
    //const int num_ctx = restPos * 3 + 1;
    const int num_ctx = (ctxSymbolMax+2)*restPos  + 1;
    const int numMaxPrefixBins = 12;
    const int numBins = 8;
    const int k = 0;
    std::cout << "--- test_encodeSymbolsSymbolsOrder1_TU" << std::endl;

    //std::vector<uint64_t> symbols(numSymbols);
    //fillVectorRandomGeometric(&symbols);
    std::srand(unsigned(std::time(nullptr)));
    std::vector<uint64_t> symbols(numSymbols);
    std::generate(symbols.begin(), symbols.end(), []() {
        return rand() % maxVal;
    });
    
    //std::vector<unsigned> symbols = {255, 0, 1, 2, 3, 4, 5, 6, 7};

    cabacSimpleSequenceEncoder binEncoder;
    binEncoder.initCtx(num_ctx, 0.5, 8);
    binEncoder.start();

    uint64_t symbolPrev = 0;
    for (unsigned int i = 0; i < symbols.size(); i++) {
        if(i > 0){
            symbolPrev = symbols[i-1];
        }
        //binEncoder.encodeBinsEG0order1(symbols[i], symbolPrev, restPos, numMaxPrefixBins);
        //binEncoder.encodeBinsEGkbypass(symbols[i], k);
        binEncoder.encodeBinsTUsymbolOrder1(symbols[i], symbolPrev, restPos, ctxSymbolMax, maxVal);
    }

    binEncoder.encodeBinTrm(1);
    binEncoder.finish();
    binEncoder.writeByteAlignment();

    std::vector<uint8_t> byteVector = binEncoder.getBitstream();

    cabacSimpleSequenceDecoder binDecoder(byteVector);
    binDecoder.initCtx(num_ctx, 0.5, 8);
    binDecoder.start();
    std::vector<uint64_t> symbolsDecoded = std::vector<uint64_t>(symbols.size(), 0);
    uint64_t symbolDecodedPrev = 0;
    for (unsigned int i = 0; i < symbolsDecoded.size(); i++) {
        if(i > 0){
            symbolDecodedPrev = symbolsDecoded[i-1];
        }
        //symbolsDecoded[i] = binDecoder.decodeBinsEG0order1(symbolDecodedPrev, restPos, numMaxPrefixBins);
        //symbolsDecoded[i] = binDecoder.decodeBinsEGkbypass(k);
        //std::cout << "Decoded symbol: " << symbolsDecoded[i] << std::endl;
        symbolsDecoded[i] = binDecoder.decodeBinsTUsymbolOrder1(symbolDecodedPrev, restPos, ctxSymbolMax, maxVal);
    }
    binDecoder.decodeBinTrm();
    binDecoder.finish();
    
    REQUIRE_THAT(symbols, Catch::Matchers::UnorderedEquals(symbolsDecoded));
}



TEST_CASE("test_encodeSymbols_function_TU")
{
    binarization::BinarizationId binId = binarization::BinarizationId::TU;
    contextSelector::ContextModelId ctxModelId = contextSelector::ContextModelId::SYMBOLORDER1;
    
    const int maxVal = 255;
    const int numSymbols = 1000; // 1000000
    const int restPos = 10;
    const int maxValCtx = 16;
    const int num_ctx = (maxValCtx+2)*restPos  + 1;

    std::vector<unsigned int> binParams = {maxVal};
    std::vector<unsigned int> ctxParams = {restPos, maxValCtx};

    std::cout << "--- test_encodeSymbols_function_TU" << std::endl;
    
    //fillVectorRandomGeometric(&symbols);
    std::srand(unsigned(std::time(nullptr)));
    std::vector<uint64_t> symbols(numSymbols);
    std::generate(symbols.begin(), symbols.end(), []() {
        return rand() % maxVal;
    });
    
    //std::vector<unsigned> symbols = {255, 0, 1, 2, 3, 4, 5, 6, 7};

    cabacSimpleSequenceEncoder binEncoder;
    binEncoder.initCtx(num_ctx, 0.5, 8);
    binEncoder.start();

    binEncoder.encodeSymbols(symbols, binId, ctxModelId, binParams, ctxParams);

    binEncoder.encodeBinTrm(1);
    binEncoder.finish();
    binEncoder.writeByteAlignment();

    std::vector<uint8_t> byteVector = binEncoder.getBitstream();

    cabacSimpleSequenceDecoder binDecoder(byteVector);
    binDecoder.initCtx(num_ctx, 0.5, 8);
    binDecoder.start();
    std::vector<uint64_t> symbolsDecoded =
        binDecoder.decodeSymbols(symbols.size(), binId, ctxModelId, binParams, ctxParams);

    binDecoder.decodeBinTrm();
    binDecoder.finish();
    
    REQUIRE_THAT(symbols, Catch::Matchers::UnorderedEquals(symbolsDecoded));
}