#if __has_include(<catch2/catch_all.hpp>)
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#else
#include <catch.hpp>
#endif

#include "ftdi.hpp"
#include "pcb_lob.hpp"
#include "simple_protocol.hpp"

TEST_CASE("", "")
{
    auto found = FtdiDriver::find_by_manufacturer_and_description("LPP", "PCB_LOB");
    if (std::size(found) > 0)
    {
        auto ftdi = FtdiDevice<Interface::A, Mode::SYNCFF, 0xff>(found[0]);
        REQUIRE(ftdi.opened());
        ftdi.set_latency_timer(250);
        ftdi.flush();
        auto decoder = make_simple_decoder<4>(4096, std::move(ftdi));
        decoder.start();
        auto data = decoder.get_samples();
        decoder.stop();
        REQUIRE(data.has_value());
    }

    {
        auto PCB = PCB_LOB(found[0]);
        REQUIRE(PCB.opened());
        PCB.start();
        for (int i = 0; i < 1000; ++i)
        {
            auto samples = PCB.samples();
            REQUIRE(samples.has_value());
        }
        PCB.stop();
    }
}
