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

#include <kodo/fulcrum/generator/random_uniform.hpp>
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
namespace fulcrum
{
namespace generator
{
struct random_uniform_wrapper : kodo::fulcrum::generator::random_uniform
{
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

void fulcrum_generator_random_uniform_generate(
    random_uniform_type& generator, pybind11::handle coefficients_handle)
{
    PyObject* coefficients_obj = coefficients_handle.ptr();

    if (!PyByteArray_Check(coefficients_obj))
    {
        throw pybind11::type_error("coefficients: expected type bytearray");
    }

    generator.generate((uint8_t*)PyByteArray_AsString(coefficients_obj));
}

void fulcrum_generator_random_uniform_generate_partial(
    random_uniform_type& generator, pybind11::handle coefficients_handle,
    std::size_t symbols)
{
    PyObject* coefficients_obj = coefficients_handle.ptr();

    if (!PyByteArray_Check(coefficients_obj))
    {
        throw pybind11::type_error("coefficients: expected type bytearray");
    }

    if (symbols > generator.symbols())
    {
        throw pybind11::value_error(
            "symbols: must be less than or equal to random_uniform.symbols()");
    }

    generator.generate_partial((uint8_t*)PyByteArray_AsString(coefficients_obj),
                               symbols);
}

void fulcrum_generator_random_uniform_generate_recode(
    random_uniform_type& generator, pybind11::handle coefficients_handle,
    const kodo_python::fulcrum::decoder_type& decoder)
{
    PyObject* coefficients_obj = coefficients_handle.ptr();

    if (!PyByteArray_Check(coefficients_obj))
    {
        throw pybind11::type_error("coefficients: expected type bytearray");
    }

    generator.generate_recode((uint8_t*)PyByteArray_AsString(coefficients_obj),
                              decoder);
}

void random_uniform(pybind11::module& m)
{
    using namespace pybind11;
    class_<random_uniform_type>(m, "RandomUniform",
                                "The Kodo uniform random fulcrum generator")
        .def(init<>(), "The random_uniform constructor\n")
        .def("configure", &random_uniform_type::configure, arg("symbols"),
             arg("expansion"),
             "Configure the generator with the given parameters. This is "
             "useful for reusing an existing coder. Note that the "
             "reconfiguration always implies a reset, so the coder will be in "
             "a clean state after this operation.\n\n"
             "\t:param symbols: The number of symbols in a coding block.\n"
             "\t:param expansion: The number of expansion symbols to use.\n")
        .def(
            "reset", &random_uniform_type::reset,
            "Resets the generator to the state when it was first configured.\n")
        .def_property_readonly("field", &random_uniform_type::field,
                               "Return the configured finite field.\n")
        .def_property_readonly(
            "symbols", &random_uniform_type::symbols,
            "Return the number of symbols supported by this generator.\n")
        .def("generate", &fulcrum_generator_random_uniform_generate,
             arg("coefficients"),
             "Generates the coefficients.\n\n"
             "\t:param coefficients: The data buffer where the coefficients "
             "will be stored.\n")
        .def("generate_partial",
             &fulcrum_generator_random_uniform_generate_partial,
             arg("coefficients"), arg("symbols"),
             "Partially generate the coefficients.\n\n"
             "\t:param coefficients: The data buffer where the "
             "coefficients "
             "will be stored.\n"
             "\t:param symbols: The number of symbols to "
             "generate coefficients "
             "for.\n"
             "\t\t Must be less than or equal to "
             "random_uniform.symbols().\n")
        .def("generate_recode",
             &fulcrum_generator_random_uniform_generate_recode,
             arg("coefficients"), arg("decoder"),
             "Generate coefficients based on the decoder state.\n\n"
             "\t:param coefficients: The data buffer where the coefficients "
             "will be stored.\n"
             "\t:param decoder: The decoder to query for the current symbol "
             "state.\n")
        .def("set_seed", &random_uniform_type::set_seed, arg("seed"),
             "Sets the state of the coefficient generator. The coefficient "
             "generator will always produce the same set of coefficients for a "
             "given seed.\n\n"
             "\t:param seed: The seed that will set the state of the "
             "generator.\n")
        .def_property_readonly("max_coefficients_bytes",
                               &random_uniform_type::max_coefficients_bytes,
                               "Return the maximum number of bytes to be "
                               "generated when calling generate.\n")
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
             "\t:param name: The chosen name for the log")
        .def("log_name", &random_uniform_type::log_name,
             "Return the log name assigned to this object.\n");
}
}
}
}
}
