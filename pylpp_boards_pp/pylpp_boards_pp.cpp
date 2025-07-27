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
        .def(py::init<>([](const std::string& serial) { return new PCB_LOB { serial }; }))
        .def_property_readonly("samples",
            [](PCB_LOB& dev) -> py::array_t<int16_t>
            {
                if(auto samples = dev.samples())
                {
                    auto& s = *samples;
                    auto arr = py::array_t<int16_t>(std::vector<ssize_t> { 4, 4096 },
                        std::vector<ssize_t> { 4096 * sizeof(int16_t), sizeof(int16_t) });
                    auto ptr = arr.mutable_unchecked<2>();
                    for (size_t i = 0; i < 4; ++i)
                    {
                        auto& sample = s[i];
                        for (size_t j = 0; j < 4096; ++j)
                        {
                            ptr(i, j) = sample[j];
                        }
                    }
                    return arr;
                }
                return py::none();
            })
        .def("start", &PCB_LOB::start)
        .def("stop", &PCB_LOB::stop)
        .def("serial_number", &PCB_LOB::serial_number)
        .def("__repr__", [](const PCB_LOB& dev)
            { return fmt::format("PCB_LOB(serial_number='{}')", dev.serial_number()); })
        .def("opened", &PCB_LOB::opened)
        .def("flush", &PCB_LOB::flush);
}
