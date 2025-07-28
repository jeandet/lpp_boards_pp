/*------------------------------------------------------------------------------
-- The MIT License (MIT)
--
-- Copyright © 2024, Laboratory of Plasma Physics- CNRS
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the “Software”), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
-- of the Software, and to permit persons to whom the Software is furnished to do
-- so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in all
-- copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
-- INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
-- PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
-- HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
-- OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
-- SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-------------------------------------------------------------------------------*/
/*-- Author : Alexis Jeandet
-- Mail : alexis.jeandet@member.fsf.org
----------------------------------------------------------------------------*/

#include <pybind11/chrono.h>
#include <pybind11/iostream.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ftdi.hpp"
#include "pcb_lob.hpp"
#include "simple_protocol.hpp"

#include <fmt/ranges.h>

namespace py = pybind11;

PYBIND11_MODULE(_pylpp_boards_pp, m, py::mod_gil_not_used())
{
    m.doc() = R"pbdoc(
        _pylpp_boards_pp
        --------
    )pbdoc";

    m.def("list_pcb_lob",
        []() { return FtdiDriver::find_by_manufacturer_and_description("LPP", "PCB_LOB"); });

    py::class_<PCB_LOB>(m, "PCB_LOB", R"pbdoc(
        PCB_LOB class
        --------
        A class to handle the PCB_LOB device.
        Attributes
        ----------
        serial_number: str
            Serial number of the device.
        )pbdoc")
        .def(py::init<>([](const std::string& serial, std::size_t samples_count=4096) { return new PCB_LOB { serial, samples_count}; }),
            py::arg("serial_number"), py::arg("samples_count") = 4096
            )
        .def_property_readonly("samples",
            [](PCB_LOB& dev) -> py::array_t<int16_t>
            {
                if (auto all_chan = dev.samples())
                {
                    auto& s = *all_chan;
                    constexpr auto channels_count = PCB_LOB::channel_count+1;

                    auto arr = py::array_t<int16_t>(
                        std::vector<ssize_t> {  static_cast<ssize_t>(s.rows()), channels_count });
                    auto ptr = arr.mutable_unchecked<2>();
                    for (std::size_t i = 0; i < channels_count; ++i)
                    {

                        for (std::size_t j = 0; j < s.rows(); ++j)
                        {
                            ptr(j,i) = s[{j,i}];
                        }
                    }
                    return arr;
                }
                return py::none();
            })
        .def("_get_raw_data",
            [](PCB_LOB& dev, py::ssize_t count)
            {
                auto arr = py::array_t<unsigned char>(
                    std::vector<ssize_t> { count }, std::vector<ssize_t> { 1 });
                auto ptr = arr.mutable_unchecked<1>();
                auto read = dev.get_raw_data(reinterpret_cast<char*>(&ptr[0]), count);
                if (static_cast<ssize_t>(read) < count)
                {
                    arr.resize({ read });
                }
                return arr;
            })
        .def("start", &PCB_LOB::start)
        .def("stop", &PCB_LOB::stop)
        .def("serial_number", &PCB_LOB::serial_number)
        .def("__repr__", [](const PCB_LOB& dev)
            { return fmt::format("PCB_LOB(serial_number='{}')", dev.serial_number()); })
        .def("opened", &PCB_LOB::opened)
        .def("flush", &PCB_LOB::flush);
}
