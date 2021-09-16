#include "catch/catch.hpp"

#include "pybind11_example/math.h"

TEST_CASE("Addition and subtraction")
{
    REQUIRE(add(1, 1) == 2);
    REQUIRE(subtract(1, 1) == 0);
}
