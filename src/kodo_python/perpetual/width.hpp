// License for Commercial Usage
// Distributed under the "KODO EVALUATION LICENSE 1.3"
//
// Licensees holding a valid commercial license may use this project
// in accordance with the standard license agreement terms provided
// with the Software (see accompanying file LICENSE.rst or
// https://www.steinwurf.com/license), unless otherwise different
// terms and conditions are agreed in writing between Licensee and
// Steinwurf ApS in which case the license will be regulated by that
// separate written agreement.
//
// License for Non-Commercial Usage
// Distributed under the "KODO RESEARCH LICENSE 1.2"
//
// Licensees holding a valid research license may use this project
// in accordance with the license agreement terms provided with the
// Software
//
// See accompanying file LICENSE.rst or https://www.steinwurf.com/license

#pragma once

#include "../version.hpp"

#include <kodo/perpetual/width.hpp>

#include <pybind11/pybind11.h>

namespace kodo_python
{
inline namespace STEINWURF_KODO_PYTHON_VERSION
{
namespace perpetual
{
namespace
{
kodo::perpetual::width from_value(uint8_t value)
{
    return static_cast<kodo::perpetual::width>(value);
}
}

void width(pybind11::module& m)
{
    pybind11::enum_<kodo::perpetual::width>(m, "Width")
        .value("_8", kodo::perpetual::width::_8, "width of 8")
        .value("_16", kodo::perpetual::width::_16, "width of 16")
        .value("_32", kodo::perpetual::width::_32, "width of 32")
        .value("_64", kodo::perpetual::width::_64, "width of 64")
        .def("from_value", &from_value);
}
}
}
}
