#ifndef CABAC_TESTS_COMMON_H_
#define CABAC_TESTS_COMMON_H_

#include <vector>
#include <cstdint>

void fillVectorRandomUniform(uint64_t min, uint64_t max, std::vector<uint64_t> *vector);
void fillVectorRandomGeometric(std::vector<uint64_t> *vector);


#endif  // GABAC_TESTS_COMMON_H_
