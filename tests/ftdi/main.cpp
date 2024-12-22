#if __has_include(<catch2/catch_all.hpp>)
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#else
#include <catch.hpp>
#endif

#include "ftdi.hpp"

TEST_CASE("", "")
{
    auto found = FtdiDriver::find_by_manufacturer_and_description("LPP", "PCB_LOB");
    REQUIRE(std::size(found) > 0);
}
