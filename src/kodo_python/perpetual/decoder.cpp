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

#include "../version.hpp"

#include <pybind11/pybind11.h>

#include <kodo/perpetual/decoder.hpp>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace kodo_python
{
inline namespace STEINWURF_KODO_PYTHON_VERSION
{
namespace perpetual
{

struct decoder_wrapper : kodo::perpetual::decoder
{
    decoder_wrapper(kodo::perpetual::width width) :
        kodo::perpetual::decoder(width)
    {
    }
    decoder_wrapper() : kodo::perpetual::decoder()
    {
    }

    std::function<void(const std::string&, const std::string&)> m_log_callback;
};

using decoder_type = decoder_wrapper;

void perpetual_decoder_enable_log(
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

void perpetual_decoder_set_symbols_storage(decoder_type& decoder,
                                           pybind11::bytearray symbols_storage)
{
    decoder.set_symbols_storage(
        (uint8_t*)PyByteArray_AsString(symbols_storage.ptr()));
}

void perpetual_decoder_decode_symbol(decoder_type& decoder,
                                     pybind11::bytearray symbol,
                                     uint64_t coefficients, std::size_t offset)
{
    if (symbol.size() < decoder.symbol_bytes())
    {
        throw pybind11::value_error(
            "symbol: not large enough to contain symbol");
    }

    if (offset >= decoder.symbols())
    {
        throw pybind11::value_error("offset: must be less than symbols");
    }

    decoder.decode_symbol((uint8_t*)PyByteArray_AsString(symbol.ptr()),
                          coefficients, offset);
}

void decoder(pybind11::module& m)
{
    using namespace pybind11;
    class_<decoder_type>(m, "Decoder", "The Kodo perpetual decoder.")
        .def(init<kodo::perpetual::width>(), arg("width"),
             "The perpetual decoder constructor\n\n"
             "\t:param width: the chosen coding width.\n")
        .def("configure", &decoder_type::configure, arg("block_bytes"),
             arg("symbol_bytes"), arg("outer_interval") = 8,
             arg("outer_segments") = 8, arg("mapping_threshold") = 0.98,
             "configure the decoder with the given parameters. This is also "
             "useful for reusing an existing coder. Note that the "
             "reconfiguration always implies a reset, so the decoder will be "
             "in a clean state after this operation.\n\n"
             "\t:param block_bytes: The size of the block in bytes.\n"
             "\t:param symbol_bytes: The size of a symbol in bytes.\n"
             "\t:param outer_interval: The number of inner code symbols "
             "between two outer code symbols\n"
             "\t:param outer_segments: The number of width segments to code "
             "outer symbols by. If outer_segments = 3 and width = 8, there are "
             "3 * 8 = 24 symbols mixed in an outer symbol"
             "\t:param mapping_threshold: The ratio of inner symbols received "
             "at which the inner code maps to the outer code\n")
        .def("reset", &decoder_type::reset, "Reset the state of the decoder.\n")
        .def_property_readonly(
            "symbols", &decoder_type::symbols,
            "Return the total number of symbols, including zero symbols, data "
            "symbols and outer symbols.\n")
        .def_property_readonly("data_symbols", &decoder_type::data_symbols,
                               "Return the number of data symbols.\n")
        .def_property_readonly("outer_symbols", &decoder_type::outer_symbols,
                               "Return the number of outer symbols.\n")
        .def_property_readonly("symbol_bytes", &decoder_type::symbol_bytes,
                               "Return the size in bytes per symbol "
                               "supported by this decoder.\n")
        .def_property_readonly(
            "width", &decoder_type::width,
            "Return the width of the encoding i.e. how many "
            "symbols are combined for each encoded inner symbol.\n")
        .def_property_readonly(
            "outer_interval", &decoder_type::outer_interval,
            "Return the interval between outer code symbols. As an example an\n"
            "interval of 8 means that there are 8 data symbols between each "
            "outer\n"
            "code symbol, if the outer interval is 0 no outer symbols exists.")
        .def_property_readonly(
            "outer_segments", &decoder_type::outer_segments,
            "Return The number of outer code segments - the outer code coding "
            "vector\n"
            "is composed of N number of Decoder.width bit segments. This also\n"
            "means that the number of mixed symbols in an outer code symbol "
            "is\n"
            "Decoder.outer_segments * Decoder.width")
        .def_property_readonly("mapping_threshold",
                               &decoder_type::mapping_threshold,
                               "Return how much of the inner code needs to be "
                               "filled before the process"
                               "of mapping to the outer code is started. The "
                               "value is in the interval ]0, 1]\n")
        .def_property_readonly("block_bytes", &decoder_type::block_bytes,
                               "Return the maximum number of bytes that can be "
                               "decoded with this decoder.\n")
        .def("set_symbols_storage", &perpetual_decoder_set_symbols_storage,
             arg("symbols_storage"),
             "Initialize all the symbols in the block.\n\n"
             "\t:param symbols_storage: The buffer containing all the data for "
             "the block.\n")
        .def("symbols_storage", &decoder_type::symbols_storage,
             "Return the memory of the block.\n")
        .def("decode_symbol", &perpetual_decoder_decode_symbol,
             arg("symbol_storage"), arg("coefficients"), arg("offset"),
             "Creates a new decoded symbol given the passed encoding "
             "coefficients.\n\n"
             "\t:param symbol_storage: This is the memory where the decoded "
             "symbol will be produced. This buffer must be large enough to "
             "store Decoder.symbol_bytes bytes.\n"
             "\t:param coefficients: These are the coding coefficients.\n"
             "\t:param offset: The offset of the coding coefficients.\n")
        .def("pivot_found", &decoder_type::pivot_found,
             "Return True if a new pivot was found during "
             "decoder::decode_symbol()."
             "Otherwise return false.\n")
        .def("pivot", &decoder_type::pivot,
             "Return the index of the newest pivot found.\n")
        .def("is_pivot", &decoder_type::is_pivot, arg("index"),
             "Return True if the decoder contains a pivot at the specific "
             "index.\n")
        .def_property_readonly("rank", &decoder_type::rank,
                               "Return the rank of the decoder.\n")
        .def("is_complete", &decoder_type::is_complete,
             "Return True if the decoder is complete, when this is true the "
             "content"
             "stored in symbols_storage is decoded.\n")
        .def(
            "enable_log", &perpetual_decoder_enable_log, arg("callback"),
            "Enable logging for the decoder.\n\n"
            "\t:param callback: The callback used for handling log messages.\n")
        .def("disable_log", &decoder_type::disable_log, "Disables the log.\n")
        .def("is_log_enabled", &decoder_type::is_log_enabled,
             "Return True if log is enabled, otherwise False.\n")
        .def("set_log_name", &decoder_type::set_log_name, arg("name"),
             "Set a log name which will be included with log messages produced "
             "by this object.\n\n"
             "\t:param name: The chosen name for the log")
        .def("log_name", &decoder_type::log_name,
             "Return the log name assigned to this object.\n");
}
}
}
}
