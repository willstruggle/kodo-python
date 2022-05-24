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

#include <kodo/slide/range.hpp>

#include <pybind11/pybind11.h>

namespace kodo_python
{
inline namespace STEINWURF_KODO_PYTHON_VERSION
{
namespace slide
{

inline kodo::slide::range py_tuple_to_range(const pybind11::tuple& tuple)
{
    PyObject* tuple_obj = tuple.ptr();
    if (tuple.size() != 2)
    {
        throw std::runtime_error("Expected a tuple of size 2");
    }
    // Check that the tuple contains integers
    if (!pybind11::isinstance<pybind11::int_>(tuple[0]) ||
        !pybind11::isinstance<pybind11::int_>(tuple[1]))
    {
        throw std::runtime_error("tuple: Expected the tuple to contain "
                                 "integers");
    }

    std::size_t lower_bound = PyLong_AsSize_t(PyTuple_GET_ITEM(tuple_obj, 0));
    std::size_t upper_bound = PyLong_AsSize_t(PyTuple_GET_ITEM(tuple_obj, 1));

    if (lower_bound > upper_bound)
    {
        throw std::runtime_error(
            "tuple: Expected the first element to be smaller than "
            "the second element");
    }

    kodo::slide::range range(lower_bound, upper_bound);

    return range;
}

}
}
}
