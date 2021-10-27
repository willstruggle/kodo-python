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

#include <pybind11/pybind11.h>

#include <kodo/block/decoder.hpp>

namespace kodo_python
{
inline namespace STEINWURF_KODO_PYTHON_VERSION
{
namespace block
{
void decoder(pybind11::module& m);

struct decoder_wrapper : kodo::block::decoder
{
    decoder_wrapper(kodo::finite_field field) : kodo::block::decoder(field)
    {
    }
    std::function<void(const std::string&, const std::string&)> m_log_callback;
};

using decoder_type = decoder_wrapper;
}
}
}
