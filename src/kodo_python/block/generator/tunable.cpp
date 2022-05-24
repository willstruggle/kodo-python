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

#include "tunable.hpp"

#include "../../version.hpp"

#include <pybind11/pybind11.h>

#include <kodo/block/generator/tunable.hpp>
#include <kodo/version.hpp>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

#include "../decoder.hpp"

namespace kodo_python
{
inline namespace STEINWURF_KODO_PYTHON_VERSION
{
namespace block
{
namespace generator
{
struct tunable_wrapper : kodo::block::generator::tunable
{
    tunable_wrapper(kodo::finite_field field) :
        kodo::block::generator::tunable(field)
    {
    }
    std::function<void(const std::string&, const std::string&)> m_log_callback;
};

using tunable_type = tunable_wrapper;

void generator_tunable_enable_log(
    tunable_type& generator,
    std::function<void(const std::string&, const std::string&)> callback)
{
    generator.m_log_callback = callback;
    generator.enable_log(
        [](const std::string& name, const std::string& message, void* data)
        {
            tunable_type* generator = static_cast<tunable_type*>(data);
            assert(generator->m_log_callback);
            generator->m_log_callback(name, message);
        },
        &generator);
}

auto block_generator_tunable_generate(tunable_type& generator, float density)
    -> pybind11::bytearray
{

    std::vector<uint8_t> coefficients(generator.max_coefficients_bytes());

    generator.generate(coefficients.data(), density);

    return pybind11::bytearray{(char*)coefficients.data(), coefficients.size()};
}

auto block_generator_tunable_generate_partial(tunable_type& generator,
                                              std::size_t symbols,
                                              float density)
    -> pybind11::bytearray
{

    if (symbols > generator.symbols())
    {
        throw pybind11::value_error(
            "symbols: must be less than or equal to tunable.symbols()");
    }

    std::vector<uint8_t> coefficients(generator.max_coefficients_bytes());

    generator.generate_partial(coefficients.data(), symbols, density);

    return pybind11::bytearray{(char*)coefficients.data(), coefficients.size()};
}

void tunable(pybind11::module& m)
{
    using namespace pybind11;
    class_<tunable_type>(m, "Tunable", "The Kodo tunable block generator")
        .def(init<kodo::finite_field>(), arg("field"),
             "The tunable constructor\n\n"
             "\t:param field: the chosen finite field.\n")
        .def("configure", &tunable_type::configure, arg("symbols"),
             "Configure the generator with the given parameters. This is "
             "useful for reusing an existing coder. Note that the "
             "reconfiguration always implies a reset, so the coder will be in "
             "a clean state after this operation.\n\n"
             "\t:param symbols: The number of symbols in a coding block.\n")
        .def(
            "reset", &tunable_type::reset,
            "Resets the generator to the state when it was first configured.\n")
        .def_property_readonly("field", &tunable_type::field,
                               "Return the configured finite field.\n")
        .def_property_readonly(
            "symbols", &tunable_type::symbols,
            "Return the number of symbols supported by this generator.\n")
        .def("generate", &block_generator_tunable_generate, arg("density"),
             "Generates the coefficients.\n\n"
             "\t:param density: A value between 1.0 and 0.0 which determines "
             "the density of the generated coefficients. The number of "
             "coefficients generated is calculated like so: max(1, "
             "symbols*density).\n")
        .def("generate_partial", &block_generator_tunable_generate_partial,
             arg("symbols"), arg("density"),
             "Partially generate the coefficients.\n\n"
             "\t:param symbols: The number of symbols to "
             "generate coefficients "
             "for. Must be less than or equal to "
             "tunable.symbols().\n"
             "\t:param density: A value between 1.0 and 0.0 which determines "
             "the density of the generated coefficients. The number of "
             "coefficients generated is calculated like so: max(1, "
             "symbols*density).\n")
        .def("set_seed", &tunable_type::set_seed, arg("seed"),
             "Sets the state of the coefficient generator. The coefficient "
             "generator will always produce the same set of coefficients for a "
             "given seed.\n\n"
             "\t:param seed: The seed that will set the state of the "
             "generator.\n")
        .def(
            "enable_log", &generator_tunable_enable_log, arg("callback"),
            "Enable logging for this generator.\n\n"
            "\t:param callback: The callback used for handling log messages.\n")
        .def("disable_log", &tunable_type::disable_log, "Disables the log.\n")
        .def("is_log_enabled", &tunable_type::is_log_enabled,
             "Return True if log is enabled, otherwise False.\n")
        .def("set_log_name", &tunable_type::set_log_name, arg("name"),
             "Set a log name which will be included with log messages produced "
             "by this object.\n\n"
             "\t:param name: The chosen name for the log")
        .def("log_name", &tunable_type::log_name,
             "Return the log name assigned to this object.\n");
}
}
}
}
}
