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

#include "random_uniform.hpp"

#include "../../version.hpp"

#include <pybind11/pybind11.h>

#include <kodo/perpetual/generator/random_uniform.hpp>

#include <cassert>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace kodo_python
{
inline namespace STEINWURF_KODO_PYTHON_VERSION
{
namespace perpetual
{
namespace generator
{
struct random_uniform_wrapper : kodo::perpetual::generator::random_uniform
{
    random_uniform_wrapper(kodo::perpetual::width width) :
        kodo::perpetual::generator::random_uniform(width)
    {
    }
    std::function<void(const std::string&, const std::string&)> m_log_callback;
};

using random_uniform_type = random_uniform_wrapper;

void generator_random_uniform_enable_log(
    random_uniform_type& generator,
    std::function<void(const std::string&, const std::string&)> callback)
{
    generator.m_log_callback = callback;
    generator.enable_log(
        [](const std::string& name, const std::string& message, void* data) {
            random_uniform_type* generator =
                static_cast<random_uniform_type*>(data);
            assert(generator->m_log_callback);
            generator->m_log_callback(name, message);
        },
        &generator);
}

void random_uniform(pybind11::module m)
{
    using namespace pybind11;
    class_<random_uniform_type>(m, "RandomUniform",
                                "The Kodo uniform random perpetual generator")
        .def(init<kodo::perpetual::width>(), arg("width"),
             "The random_uniform constructor\n\n"
             "\t:param width: the chosen coding width.\n")
        .def_property_readonly(
            "width", &random_uniform_type::width,
            "Return the width of the generator in symbols.\n")
        .def("generate", &random_uniform_type::generate, arg("seed"),
             "Generates the coefficients.\n\n"
             "Return the generated coefficients as an integer"
             "where each bit represents a coefficient.\n"
             "\t:param seed: The seed for the generator.\n")
        .def(
            "enable_log", &generator_random_uniform_enable_log, arg("callback"),
            "Enable logging for this generator.\n\n"
            "\t:param callback: The callback used for handling log messages.\n")
        .def("disable_log", &random_uniform_type::disable_log,
             "Disables the log.\n")
        .def("is_log_enabled", &random_uniform_type::is_log_enabled,
             "Return True if log is enabled, otherwise False.\n")

        .def("set_log_name", &random_uniform_type::set_log_name, arg("name"),
             "Set a log name which will be included with log messages produced "
             "by this object.\n\n"
             "\t:param name: The chosen name for the log\n")
        .def("log_name", &random_uniform_type::log_name,
             "Return the log name assigned to this object.\n");
    ;
}
}
}
}
}
