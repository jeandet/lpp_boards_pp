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
    if (std::size(found) > 0)
    {
        auto ftdi = FtdiDevice<Interface::A, Mode::SYNCFF, 0xff>(found[0]);
        REQUIRE(ftdi.opened());
        ftdi.set_latency_timer(250);
        ftdi.flush_buffers();
        auto data = ftdi.read(64);
        REQUIRE(std::size(data) == 64);
    }
}
