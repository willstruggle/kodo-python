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

#include "encoder.hpp"
#include "tuple_to_range.hpp"

#include "../version.hpp"

#include <pybind11/pybind11.h>

#include <kodo/slide/encoder.hpp>
#include <kodo/slide/stream.hpp>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace kodo_python
{
inline namespace STEINWURF_KODO_PYTHON_VERSION
{
namespace slide
{
namespace
{
struct encoder_wrapper : kodo::slide::encoder
{
    encoder_wrapper(kodo::finite_field field) : kodo::slide::encoder(field)
    {
    }
    std::function<void(const std::string&, const std::string&)> m_log_callback;
    bool configured = false;

    ~encoder_wrapper()
    {
        if (configured)
        {
            reset(
                [](uint64_t index, const uint8_t* symbol, void* user_data)
                {
                    (void)user_data;
                    (void)index;
                    delete[] symbol;
                },
                nullptr);
        }
    }
};
using encoder_type = encoder_wrapper;

void slide_encoder_enable_log(
    encoder_type& encoder,
    std::function<void(const std::string&, const std::string&)> callback)
{
    encoder.m_log_callback = callback;
    encoder.enable_log(
        [](const std::string& name, const std::string& message, void* data)
        {
            encoder_type* encoder = static_cast<encoder_type*>(data);
            assert(encoder->m_log_callback);
            encoder->m_log_callback(name, message);
        },
        &encoder);
}

void slide_encoder_reset(encoder_type& encoder)
{
    encoder.reset(
        [](uint64_t index, const uint8_t* symbol, void* user_data)
        {
            (void)user_data;
            (void)index;
            delete[] symbol;
        },
        nullptr);
}

void slide_encoder_configure(encoder_type& encoder,
                             std::size_t max_symbol_bytes)
{
    if (encoder.configured)
    {
        slide_encoder_reset(encoder);
    }
    encoder.configure(max_symbol_bytes);
    encoder.configured = true;
}

auto slide_encoder_stream_range(encoder_type& encoder) -> pybind11::tuple
{
    kodo::slide::range range = encoder.stream_range();
    pybind11::tuple stream_tuple =
        pybind11::make_tuple(range.lower_bound(), range.upper_bound());
    return stream_tuple;
}

void slide_encoder_push_symbol(encoder_type& encoder,
                               pybind11::bytearray symbol_bytearray)
{
    auto size = symbol_bytearray.size();
    if (size > encoder.max_symbol_bytes())
    {
        throw pybind11::value_error(
            "symbol: greater than encoder.max_symbol_bytes(). Must be less "
            "than or equal");
    }

    uint8_t* symbol = new uint8_t[size];
    std::memcpy((uint8_t*)PyByteArray_AsString(symbol_bytearray.ptr()), symbol,
                size);
    encoder.push_symbol(symbol, size);
}

auto slide_encoder_symbol_data(encoder_type& encoder, std::size_t index)
    -> pybind11::bytearray
{
    if (!encoder.in_stream(index))
        throw pybind11::value_error("index not in stream");

    auto symbol = encoder.symbol_data(index);
    auto size = encoder.symbol_bytes(index);

    return pybind11::bytearray{(char*)symbol, size};
}

auto slide_encoder_in_stream(encoder_type& encoder, std::size_t index) -> bool
{
    return encoder.in_stream(index);
}

void slide_encoder_pop_symbol(encoder_type& encoder)
{
    if (encoder.is_stream_empty())
        throw pybind11::value_error("Stream was empty");

    auto symbol = encoder.pop_symbol();
    delete[] symbol;
}

auto slide_encoder_encode_symbol(encoder_type& encoder,
                                 pybind11::tuple window_handle,
                                 pybind11::bytearray coefficients)
    -> pybind11::bytearray
{
    if (coefficients.size() == 0)
    {
        throw pybind11::value_error("coefficients: length is 0.");
    }

    kodo::slide::range range = py_tuple_to_range(window_handle);

    std::vector<uint8_t> symbol(encoder.max_symbol_bytes());

    auto size = encoder.encode_symbol(
        symbol.data(), range,
        (uint8_t*)PyByteArray_AsString(coefficients.ptr()));

    return pybind11::bytearray{(char*)symbol.data(), size};
}

auto slide_encoder_encode_systematic_symbol(encoder_type& encoder,
                                            std::size_t index)
    -> pybind11::bytearray
{
    if (!encoder.in_stream(index))
    {
        throw pybind11::value_error("index: out of range, must be in stream.");
    }

    std::vector<uint8_t> symbol(encoder.symbol_bytes(index));
    auto size = encoder.encode_systematic_symbol(symbol.data(), index);

    return pybind11::bytearray{(char*)symbol.data(), size};
}
}

void encoder(pybind11::module& m)
{
    using namespace pybind11;
    class_<encoder_type>(m, "Encoder", "The Kodo sliding window encoder")
        .def(init<kodo::finite_field>(), arg("field"),
             "The sliding window encoder constructor\n\n"
             "\t:param field: the chosen finite field.\n")
        .def("configure", &slide_encoder_configure, arg("max_symbol_bytes"),
             "Configures the encoder with the given parameters. This must"
             "be called before anything else. If needed configure can be"
             "called again. This is useful for reusing an existing coder."
             "Note that the a reconfiguration always implies a reset,"
             "so the coder will be in a clean state after the operation\n\n"
             "\t:param max_symbol_bytes: The size of a symbol in bytes.\n")
        .def("reset", &slide_encoder_reset, "Reset the state of the encoder.\n")
        .def_property_readonly("field", &encoder_type::field,
                               "Return the finite field used.\n")
        .def_property_readonly(
            "max_symbol_bytes", &encoder_type::max_symbol_bytes,
            "Return the maximum size of a symbol in the stream in bytes.\n")
        .def_property_readonly(
            "stream_symbols", &encoder_type::stream_symbols,
            "Return the total number of symbols available in memory at the "
            "encoder."
            "The number of symbols in the coding window MUST be less than"
            "or equal to this number.\n")
        .def_property_readonly(
            "is_stream_empty", &encoder_type::is_stream_empty,
            "Return True if Encoder.stream_symbols == 0. Else False.\n")
        .def_property_readonly(
            "stream_lower_bound", &encoder_type::stream_lower_bound,
            "Return the index of the oldest symbol known by the encoder. "
            "This "
            "symbol"
            "may not be inside the window but can be included in the window"
            "if needed.\n")
        .def_property_readonly("stream_upper_bound",
                               &encoder_type::stream_upper_bound,
                               "Return the upper bound of the stream.\n")
        .def_property_readonly("stream_range", &slide_encoder_stream_range,
                               "Return a tuple containing the lower- and upper "
                               "bound of the stream.\n")
        .def("in_stream", &encoder_type::in_stream, arg("index"),
             "Return True if the stream contains a symbol with the given "
             "index, otherwise False.\n\n"
             ":param index: The index to look for in the stream.\n")
        .def_property_readonly("stream_upper_bound",
                               &encoder_type::stream_upper_bound,
                               "Return the upper bound of the stream.\n")
        .def("push_symbol", &slide_encoder_push_symbol, arg("symbol"),
             "Adds a new symbol to the front of the encoder. Increments the "
             "number"
             "of symbols in the stream and increases the"
             "Encoder.stream_upper_bound\n\n"
             ":param symbol: The buffer containing all the data for the symbol"
             "Return the stream index of the symbol being added.\n")
        .def("pop_symbol", &slide_encoder_pop_symbol,
             "Remove the \"oldest\" symbol from the stream. Increments the"
             "Encoder.stream_lower_bound\n\n"
             "Return the index of the symbol being removed.\n")
        .def(
            "encode_symbol", &slide_encoder_encode_symbol, arg("window"),
            arg("coefficients"),
            "Return an encoded symbol according to the coding coefficients and "
            "window.\n\n"
            ":param coefficients: The coding coefficients. These must have the"
            "\tmemory layout required (see README.rst). A compatible format can"
            "\tbe created using Encoder.generate()."
            "Return The size of the output symbol in bytes i.e the number of"
            "\tbytes used from the symbol buffer.\n")
        .def("encode_systematic_symbol",
             &slide_encoder_encode_systematic_symbol, arg("index"),
             "Return a source symbol bytearray.\n\n"
             ":param index: The symbol index which should be written."
             "Return The size of the output symbol in bytes i.e the number of"
             "\tbytes used from the symbol buffer.")
        .def("symbol_data", &slide_encoder_symbol_data, arg("index"),
             "Get a symbol from the stream.\n\n"
             ":param index: The index of the symbol to get.")
        .def("in_stream", &slide_encoder_in_stream, arg("index"),
             "Check if a symbol is contained in the stream.\n\n"
             ":param index: The index of the symbol to check.")
        .def("enable_log", &slide_encoder_enable_log, arg("callback"),
             "Enable logging for this encoder.\n\n"
             ":param callback: The callback used for handling "
             "log messages.")
        .def("disable_log", &encoder_type::disable_log, "Disables the log.\n")
        .def("is_log_enabled", &encoder_type::is_log_enabled,
             "Return True if log is enabled, otherwise False.\n")
        .def("set_log_name", &encoder_type::set_log_name, arg("name"),
             "Set a log name which will be included with log messages produced "
             "by this object.\n\n"
             "\t:param name: The chosen name for the log")
        .def("log_name", &encoder_type::log_name,
             "Return the log name assigned to this object.\n");
}
}
}
}
