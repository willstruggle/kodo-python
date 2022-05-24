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

#include <kodo/block/decoder.hpp>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace kodo_python
{
inline namespace STEINWURF_KODO_PYTHON_VERSION
{
namespace block
{

void block_decoder_enable_log(
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

void block_decoder_decode_symbol(decoder_type& decoder,
                                 pybind11::bytearray symbol,
                                 pybind11::bytearray coefficients)
{

    if (symbol.size() < decoder.symbol_bytes())
    {
        throw pybind11::value_error(
            "symbol: not large enough to contain symbol");
    }

    decoder.decode_symbol((uint8_t*)PyByteArray_AsString(symbol.ptr()),
                          (uint8_t*)PyByteArray_AsString(coefficients.ptr()));
}

void block_decoder_decode_systematic_symbol(decoder_type& decoder,
                                            pybind11::bytearray symbol,
                                            std::size_t index)
{

    if (index >= decoder.symbols())
    {
        throw pybind11::value_error("index: must be less than symbols");
    }

    decoder.decode_systematic_symbol(
        (const uint8_t*)PyByteArray_AsString(symbol.ptr()), index);
}

void block_decoder_set_symbols_storage(decoder_type& decoder,
                                       pybind11::bytearray symbols_storage)
{
    decoder.set_symbols_storage(
        (uint8_t*)PyByteArray_AsString(symbols_storage.ptr()));
}

void block_decoder_set_symbol_storage(decoder_type& decoder,
                                      pybind11::bytearray symbol_storage,
                                      std::size_t index)
{
    if (index >= decoder.symbols())
    {
        throw pybind11::value_error("index: must be less than symbols");
    }

    decoder.set_symbol_storage(
        (uint8_t*)PyByteArray_AsString(symbol_storage.ptr()), index);
}

void block_decoder_recode_symbol(decoder_type& decoder,
                                 pybind11::bytearray symbol,
                                 pybind11::bytearray coefficients,
                                 pybind11::bytearray coefficients_in)
{
    if (symbol.size() < decoder.symbol_bytes())
    {
        throw pybind11::value_error(
            "symbol: not large enough to contain symbol");
    }

    decoder.recode_symbol(
        (uint8_t*)PyByteArray_AsString(symbol.ptr()),
        (uint8_t*)PyByteArray_AsString(coefficients.ptr()),
        (const uint8_t*)PyByteArray_AsString(coefficients_in.ptr()));
}

auto block_decoder_symbol_data(decoder_type& decoder, std::size_t index)
    -> pybind11::bytearray
{
    if (index >= decoder.symbols())
    {
        throw pybind11::value_error("index: must be less than symbols");
    }
    auto symbol = decoder.symbol_data(index);
    return pybind11::bytearray{(char*)symbol, decoder.symbol_bytes()};
}

void decoder(pybind11::module& m)
{
    using namespace pybind11;
    class_<decoder_type>(m, "Decoder", "The Kodo block decoder")
        .def(init<kodo::finite_field>(), arg("field"),
             "The block decoder constructor\n\n"
             "\t:param field: the chosen finite field.\n")
        .def("configure", &decoder_type::configure, arg("symbols"),
             arg("symbol_bytes"),
             "Configure the decoder with the given parameters. This is also "
             "useful for reusing an existing coder. Note that the "
             "reconfiguration always implies a reset, so the decoder will be "
             "in a clean state after this operation.\n\n"
             "\t:param symbols: The number of symbols.\n"
             "\t:param symbol_bytes: The size of a symbol in bytes.\n")
        .def("reset", &decoder_type::reset, "Reset the state of the decoder.\n")
        .def_property_readonly(
            "symbols", &decoder_type::symbols,
            "Return the number of symbols supported by this decoder.\n")
        .def_property_readonly(
            "symbol_bytes", &decoder_type::symbol_bytes,
            "Return the size in bytes per symbol supported by this decoder.\n")
        .def_property_readonly("field", &decoder_type::field,
                               "Return the :class:`~kodo.FiniteField` used.\n")
        .def("set_symbols_storage", &block_decoder_set_symbols_storage,
             arg("symbols_storage"),
             "Initialize all the symbols in the block.\n\n"
             "\tparam symbols_storage: The buffer containing all the data for "
             "the the block.")
        .def("set_symbol_storage", &block_decoder_set_symbol_storage,
             arg("symbol_storage"), arg("index"),
             "Set a symbol to be encoded.\n\n"
             "\t:param symbol_storage: The buffer containing all the data for "
             "the block.\n"
             "\t:param index: The index of the symbol. Note the index must be "
             "equal to Decoder.rank() + 1.\n")
        .def("symbol_data", &block_decoder_symbol_data, arg("index"),
             "Get the memory for a symbol.\n\n"
             "\t:param index: The index of the symbol to get.\n")
        .def_property_readonly("block_bytes", &decoder_type::block_bytes,
                               "Return the total number of bytes that can be "
                               "encoded with this decoder.\n")
        .def("decode_symbol", &block_decoder_decode_symbol, arg("symbol"),
             arg("coefficients"),
             "Feed a coded symbol to the decoder.\n\n"
             "\t:param symbol: The data of the symbol assumed to be "
             "symbol_bytes() bytes in size.\n"
             "\t:param coefficients: The coding coefficients that describe the "
             "encoding performed on the symbol.\n")
        .def("decode_systematic_symbol",
             &block_decoder_decode_systematic_symbol, arg("symbol"),
             arg("index"),
             "Feed a systematic, i.e, un-coded symbol to the decoder.\n\n"
             "\t:param symbol: The data of the symbol assumed to be "
             "symbol_bytes() bytes in size.\n"
             "\t:param index: The index of the given symbol.")
        .def_property_readonly("rank", &decoder_type::rank,
                               "Return the rank of the decoder.\n")
        .def("recode_symbol", &block_decoder_recode_symbol,
             arg("symbol_storage"), arg("coefficients"), arg("coefficients_in"),
             "Recodes a new encoded symbol given the passed encoding "
             "coeffcients. Coefficient values for symbols not seen must be 0. "
             "This can be ensured, e.g., by using the RandomUniform "
             "generator's generate_recode() function.\n\n"
             ":param symbol: This is the memory where the encoded symbol will "
             "be produced. This buffer must be large enough to store "
             "symbol_bytes() bytes.\n"
             ":param coefficients: These are the coding coefficients.\n"
             ":param coefficients_in: These are the input coding "
             "coefficients.\n")
        .def("is_symbol_pivot", &decoder_type::is_symbol_pivot, arg("index"),
             "Return True if the decoder contains a pivot at the specific "
             "index.\n")
        .def("is_symbol_decoded", &decoder_type::is_symbol_decoded,
             arg("index"),
             "Return True if the decoder contains a decoded symbol at the "
             "specific index.\n")
        .def("is_complete", &decoder_type::is_complete,
             "Return True if the decoder is complete, when this is true the "
             "content stored in symbols_storage is decoded.\n")
        .def(
            "enable_log", &block_decoder_enable_log, arg("callback"),
            "Enable logging for this decoder.\n\n"
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
