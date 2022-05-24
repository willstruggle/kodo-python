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

#include "decoder.hpp"
#include "tuple_to_range.hpp"

#include "../version.hpp"

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include <kodo/slide/decoder.hpp>
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
struct decoder_wrapper : kodo::slide::decoder
{
    decoder_wrapper(kodo::finite_field field) : kodo::slide::decoder(field)
    {
    }
    std::function<void(const std::string&, const std::string&)> m_log_callback;
    std::function<void(uint64_t index)> m_on_symbol_decoded;

    bool configured = false;

    ~decoder_wrapper()
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
    };
};

using decoder_type = decoder_wrapper;

void slide_decoder_enable_log(
    decoder_type& decoder,
    std::function<void(const std::string&, const std::string&)> callback)
{
    decoder.m_log_callback = callback;
    decoder.enable_log(
        [](const std::string& name, const std::string& message, void* data)
        {
            decoder_type* decoder = static_cast<decoder_type*>(data);
            assert(decoder->m_log_callback);
            decoder->m_log_callback(name, message);
        },
        &decoder);
}

void slide_decoder_on_symbol_decoded(
    decoder_type& decoder,
    std::function<void(uint64_t index)> on_symbol_decoded)
{
    decoder.m_on_symbol_decoded = on_symbol_decoded;
    decoder.on_symbol_decoded(
        [](uint64_t index, void* user_data)
        {
            decoder_type* decoder = static_cast<decoder_type*>(user_data);
            assert(decoder->m_on_symbol_decoded);
            decoder->m_on_symbol_decoded(index);
        },
        &decoder);
}

void slide_decoder_reset(decoder_type& decoder)
{

    decoder.reset(
        [](uint64_t index, const uint8_t* symbol, void* user_data)
        {
            (void)user_data;
            (void)index;
            delete[] symbol;
        },
        nullptr);
}

void slide_decoder_configure(decoder_type& decoder,
                             std::size_t max_max_symbol_bytes)
{
    if (decoder.configured)
    {
        slide_decoder_reset(decoder);
    }
    decoder.configure(max_max_symbol_bytes);
    decoder.configured = true;
}

auto slide_decoder_stream_range(decoder_type& decoder) -> pybind11::tuple
{
    kodo::slide::range range = decoder.stream_range();
    pybind11::tuple stream_tuple =
        pybind11::make_tuple(range.lower_bound(), range.upper_bound());
    return stream_tuple;
}

void slide_decoder_push_symbol(decoder_type& decoder)
{
    decoder.push_symbol();
}

void slide_decoder_pop_symbol(decoder_type& decoder)
{
    assert(decoder.stream_symbols() != 0);
    auto ptr = decoder.pop_symbol();
    if (ptr == nullptr)
    {
        return;
    }

    delete[] ptr;
}

auto slide_decoder_symbol_data(decoder_type& decoder, std::size_t index)
    -> pybind11::bytearray
{
    auto symbol = decoder.symbol_data(index);
    auto size = decoder.symbol_bytes(index);
    if (!decoder.in_stream(index))
    {
        return pybind11::none{};
    }

    return pybind11::bytearray{(char*)symbol, size};
}

void slide_decoder_decode_symbol(decoder_type& decoder,
                                 pybind11::bytearray symbol_bytearray,
                                 pybind11::tuple window_tuple,
                                 pybind11::bytearray coefficients)
{
    if (coefficients.size() == 0)
    {
        throw pybind11::value_error("coefficients: length is 0.");
    }

    kodo::slide::range range = py_tuple_to_range(window_tuple);

    auto size = symbol_bytearray.size();

    uint8_t* symbol = new uint8_t[decoder.max_symbol_bytes()];
    memcpy(symbol, (uint8_t*)PyByteArray_AsString(symbol_bytearray.ptr()),
           size);

    auto ptr = decoder.decode_symbol(
        symbol, size, range,
        (uint8_t*)PyByteArray_AsString(coefficients.ptr()));

    if (ptr == nullptr)
    {
        return;
    }

    delete[] ptr;
}

void slide_decoder_decode_systematic_symbol(
    decoder_type& decoder, pybind11::bytearray symbol_bytearray,
    std::size_t index)
{
    if (symbol_bytearray.size() > decoder.max_symbol_bytes())
    {
        throw pybind11::value_error(
            "symbol: bytearray too large. Must be less "
            "than or equal to decoder.max_symbol_bytes");
    }

    if (!decoder.in_stream(index))
    {
        throw pybind11::value_error("index: out of range, must be in stream.");
    }

    auto size = symbol_bytearray.size();

    uint8_t* symbol = new uint8_t[decoder.max_symbol_bytes()];

    memcpy(symbol, (uint8_t*)PyByteArray_AsString(symbol_bytearray.ptr()),
           size);

    auto ptr = decoder.decode_systematic_symbol(symbol, size, index);

    if (ptr == nullptr)
    {
        return;
    }

    delete[] ptr;
}
}

void decoder(pybind11::module& m)
{
    using namespace pybind11;
    class_<decoder_type>(m, "Decoder", "The Kodo sliding window decoder")
        .def(init<kodo::finite_field>(), arg("field"),
             "The sliding window decoder constructor\n\n"
             "\t:param field: the chosen finite field.\n")
        .def("configure", &slide_decoder_configure, arg("max_symbol_bytes"),
             "Configure the decoder with the given parameters. This is also "
             "useful for reusing an existing coder. Note that the "
             "reconfiguration always implies a reset, so the decoder will "
             "be in a clean state after this operation.\n\n"
             "\t:param max_symbol_bytes: The size of a symbol in bytes.\n")
        .def("reset", &slide_decoder_reset, "Reset the state of the decoder.\n")
        .def_property_readonly("field", &decoder_type::field,
                               "Return the finite field used.\n")
        .def_property_readonly(
            "max_symbol_bytes", &decoder_type::max_symbol_bytes,
            "Return the size of a symbol in the stream in bytes.\n")
        .def_property_readonly(
            "stream_symbols", &decoder_type::stream_symbols,
            "Return the total number of symbols known at the "
            "decoder."
            "The number of symbols in the coding window MUST be less than"
            "or equal to this number.\n")
        .def_property_readonly(
            "is_stream_empty", &decoder_type::is_stream_empty,
            "Return True if Decoder.stream_symbols == 0. Else False.\n")
        .def_property_readonly(
            "stream_lower_bound", &decoder_type::stream_lower_bound,
            "Return the index of the oldest symbol known by the decoder. "
            "This symbol may not be inside the window but can be included in "
            "the window if needed.\n")
        .def_property_readonly("stream_upper_bound",
                               &decoder_type::stream_upper_bound,
                               "Return the upper bound of the stream.\n")
        .def_property_readonly("stream_range", &slide_decoder_stream_range,
                               "Return a tuple containing the lower- and upper "
                               "bound of the stream.\n")
        .def("in_stream", &decoder_type::in_stream, arg("index"),
             "Return True if the stream contains a symbol with the given "
             "index, otherwise False.\n\n"
             ":param index: The index to look for in the stream.\n")
        .def("set_stream_lower_bound", &decoder_type::set_stream_lower_bound,
             arg("stream_lower_bound"),
             "Set the index of the oldest symbol known by the decoder."
             "This must only be called on a decoder with an empty "
             "stream.\n\n"
             ":param stream_lower_bound: The new stream lower bound, the new "
             "lower bound must be larger than or equal to the prior lower "
             "bound.\n")
        .def("push_symbol", &slide_decoder_push_symbol,
             "Adds a new symbol to the front of the decoder. Increments the "
             "number of symbols in the stream and increases the "
             "Decoder.stream_upper_bound\n")
        .def("pop_symbol", &slide_decoder_pop_symbol,
             "Remove the \"oldest\" symbol from the stream. Increments the"
             "Decoder.stream_lower_bound\n")
        .def("decode_symbol", &slide_decoder_decode_symbol, arg("symbol"),
             arg("window"), arg("coefficients"),
             "Write an decoded symbol according to the coding "
             "coefficients.\n\n"
             ":param symbol: The buffer where the decoded symbol will be "
             "stored. The"
             "\tsymbol must be Decoder.max_symbol_bytes large."
             ":param window: A tuple of two integers. A lower bound and an "
             "upper_bound of the packet indices included in the encoded symbol."
             ":param coefficients: The coding coefficients. These must "
             "have the"
             "\tmemory layout required (see README.rst). A compatible "
             "format can"
             "\tbe created using Decoder.generate().\n")
        .def("decode_systematic_symbol",
             &slide_decoder_decode_systematic_symbol, arg("symbol"),
             arg("index"),
             "Write a source symbol to the symbol buffer.\n\n"
             ":param symbol: The buffer where the source symbol will be "
             "stored. The"
             "\tsymbol buffer must be Decoder.max_symbol_bytes large"
             ":param index: The symbol index which should be written.\n")
        .def_property_readonly(
            "rank", &decoder_type::rank,
            "The rank of a decoder indicates how many symbols have been"
            "partially or fully decoded. This number is equivalent to the"
            "number of pivot elements in the stream.\n\n"
            "Return the rank of the decoder.\n")
        .def_property_readonly(
            "symbols_missing", &decoder_type::symbols_missing,
            "Return the number of missing symbols in the stream.\n")
        .def_property_readonly("symbols_partially_decoded",
                               &decoder_type::symbols_partially_decoded,
                               "Return the number of partially decoded "
                               "symbols in the stream.\n")
        .def_property_readonly(
            "symbols_decoded", &decoder_type::symbols_decoded,
            "Return the number of decoded symbols in the stream.\n")
        .def("symbol_data", &slide_decoder_symbol_data, arg("index"),
             "Get a symbol from the stream.\n\n"
             ":param index: The index of the symbol to get.")
        .def("is_symbol_decoded", &decoder_type::is_symbol_decoded,
             arg("index"),
             ":param index: The index of the symbol to check.\n\n"
             "Return True if the symbol is decoded (i.e it correspond to a "
             "source"
             "\tsymbol), otherwise False.\n")
        .def("on_symbol_decoded", &slide_decoder_on_symbol_decoded,
             arg("decoding_callback"),
             "Sets a callback to be executed when a symbol is decoded.\n\n"
             ":param decoding_callback: A function that takes an index and has "
             "no return value.\n")
        .def("enable_log", &slide_decoder_enable_log, arg("callback"),
             "Enable logging for this decoder.\n\n"
             ":param callback: The callback used for handling "
             "log messages.")
        .def("disable_log", &decoder_type::disable_log, "Disables the log.\n")
        .def("is_log_enabled", &decoder_type::is_log_enabled,
             "Return True if log is enabled, otherwise False.\n")
        .def("set_log_name", &decoder_type::set_log_name, arg("name"),
             "Set a log name which will be included with log messages "
             "produced "
             "by this object.\n\n"
             "\t:param name: The chosen name for the log")
        .def("log_name", &decoder_type::log_name,
             "Return the log name assigned to this object.\n");
}
}
}
}
