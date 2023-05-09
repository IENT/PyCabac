#ifndef CABAC_TESTS_COMMON_H_
#define CABAC_TESTS_COMMON_H_

#include <vector>
#include <cstdint>

void fillVectorRandomUniform(uint64_t min, uint64_t max, std::vector<unsigned int> *vector);
void fillVectorRandomGeometric(std::vector<unsigned int> *vector);


#endif  // GABAC_TESTS_COMMON_H_
