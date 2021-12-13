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

#include "../version.hpp"

#include <pybind11/pybind11.h>

#include <kodo/perpetual/encoder.hpp>

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

struct encoder_wrapper : kodo::perpetual::encoder
{
    encoder_wrapper(kodo::perpetual::width width) :
        kodo::perpetual::encoder(width)
    {
    }
    encoder_wrapper() : kodo::perpetual::encoder()
    {
    }
    std::function<void(const std::string&, const std::string&)> m_log_callback;
};

using encoder_type = encoder_wrapper;

void perpetual_encoder_enable_log(
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

void perpetual_encoder_set_symbols_storage(
    encoder_type& encoder, pybind11::handle symbols_storage_handle)
{
    PyObject* symbols_storage_obj = symbols_storage_handle.ptr();

    if (!PyByteArray_Check(symbols_storage_obj))
    {
        throw pybind11::type_error("symbols_storage: expected type bytearray");
    }

    encoder.set_symbols_storage(
        (uint8_t*)PyByteArray_AsString(symbols_storage_obj));
}

void perpetual_encoder_encode_symbol(encoder_type& encoder,
                                     pybind11::handle symbol_handle,
                                     uint64_t coefficients, std::size_t offset)
{
    PyObject* symbol_obj = symbol_handle.ptr();

    if (!PyByteArray_Check(symbol_obj))
    {
        throw pybind11::type_error("symbol: expected type bytearray");
    }

    if ((std::size_t)PyByteArray_Size(symbol_obj) < encoder.symbol_bytes())
    {
        throw pybind11::value_error(
            "symbol: not large enough to contain symbol");
    }

    if (offset >= encoder.symbols())
    {
        throw pybind11::value_error("offset: must be less than symbols");
    }

    encoder.encode_symbol((uint8_t*)PyByteArray_AsString(symbol_obj),
                          coefficients, offset);
}

void encoder(pybind11::module& m)
{
    using namespace pybind11;
    class_<encoder_type>(m, "Encoder", "The Kodo perpetual encoder")
        .def(init<kodo::perpetual::width>(), arg("width"),
             "The perpetual encoder constructor\n\n"
             "\t:param width: the chosen coding width.\n")
        .def("configure", &encoder_type::configure, arg("block_bytes"),
             arg("symbol_bytes"), arg("outer_interval") = 8,
             arg("outer_segments") = 8,
             "configure the encoder with the given parameters. This is also "
             "useful for reusing an existing coder. Note that the "
             "reconfiguration always implies a reset, so the encoder will be "
             "in a clean state after this operation.\n\n"
             "\t:param block_bytes: The size of the block in bytes.\n"
             "\t:param symbol_bytes: The size of a symbol in bytes.\n"
             "\t:param outer_interval: The number of inner symbols between two"
             "outer code symbols. If the outer interval is 0 no outer symbols "
             "exists.\n"
             "\t:param outer_segments: The number of width segments to code "
             "outer symbols by. If outer_segments = 3 and width = 8, there are "
             "3 * 8 = 24 symbols mixed in an outer symbol\n")
        .def("reset", &encoder_type::reset, "Reset the state of the encoder.\n")
        .def_property_readonly(
            "symbols", &encoder_type::symbols,
            "Return the total number of symbols, including zero symbols, data "
            "symbols and outer symbols.\n")
        .def_property_readonly("data_symbols", &encoder_type::data_symbols,
                               "Return the number of data symbols.\n")
        .def_property_readonly("outer_symbols", &encoder_type::outer_symbols,
                               "Return the number of outer symbols.\n")
        .def_property_readonly("symbol_bytes", &encoder_type::symbol_bytes,
                               "Return the size in bytes per symbol "
                               "supported by this encoder.\n")
        .def_property_readonly(
            "width", &encoder_type::width,
            "Return the width of the encoding i.e. how many "
            "symbols are combined for each encoded inner symbol.\n")
        .def_property_readonly(
            "outer_interval", &encoder_type::outer_interval,
            "Return the interval between outer code symbols. As an example an\n"
            "interval of 8 means that there are 8 data symbols between each "
            "outer\n"
            "code symbol, if the outer interval is 0 no outer symbols exists.")
        .def_property_readonly(
            "outer_segments", &encoder_type::outer_segments,
            "Return The number of outer code segments - the outer code coding "
            "vector\n"
            "is composed of N number of Decoder.width bit segments. This also\n"
            "means that the number of mixed symbols in an outer code symbol "
            "is\n"
            "Decoder.outer_segments * Decoder.width")
        .def_property_readonly("block_bytes", &encoder_type::block_bytes,
                               "Return the maximum number of bytes that can be "
                               "encoded with this encoder.\n")
        .def("set_symbols_storage", &perpetual_encoder_set_symbols_storage,
             arg("symbols_storage"),
             "Initialize all the symbols in the block.\n\n"
             "\t:param symbols_storage: The buffer containing all the data for "
             "the block.\n")
        .def("symbols_storage", &encoder_type::symbols_storage,
             "Return the memory of the block.\n")
        .def("encode_symbol", &perpetual_encoder_encode_symbol,
             arg("symbol_storage"), arg("coefficients"), arg("offset"),
             "Creates a new encoded symbol given the passed encoding "
             "coefficients.\n\n"
             "\t:param symbol_storage: This is the memory where the encoded "
             "symbol will be produced. This buffer must be large enough to "
             "store Encoder.symbol_bytes bytes.\n"
             "\t:param coefficients: These are the coding coefficients.\n"
             "\t:param offset: The offset of the coding coefficients.\n")
        .def(
            "enable_log", &perpetual_encoder_enable_log, arg("callback"),
            "Enable logging for the encoder.\n\n"
            "\t:param callback: The callback used for handling log messages.\n")
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
