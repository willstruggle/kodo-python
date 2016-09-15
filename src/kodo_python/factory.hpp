// Copyright Steinwurf ApS 2015.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#pragma once

#include <string>

#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include <kodo_core/has_is_complete.hpp>
#include "resolve_field_name.hpp"

namespace kodo_python
{
    template<template<class, class> class Coder>
    struct extra_factory_methods
    {
        template<class FactoryClass>
        extra_factory_methods(FactoryClass& factory_class)
        {
            (void) factory_class;
        }
    };

    template<
        template<class, class> class Coder,
        class Field, class TraceTag
    >
    void factory(const std::string& stack)
    {
        using boost::python::arg;
        using boost::python::args;
        using boost::python::class_;
        using boost::python::init;
        using stack_type = Coder<Field, TraceTag>;
        using factory_type = typename stack_type::factory;

        std::string field = resolve_field_name<Field>();
        std::string coder =
            kodo_core::has_is_complete<stack_type>::value ? "Decoder" : "Encoder";
        std::string kind = coder + std::string("Factory");
        std::string name = stack + kind + field;

        auto factory_class =
            class_<factory_type, boost::noncopyable>(
                name.c_str(),
                "Factory for creating encoders/decoders.",
                init<uint32_t, uint32_t>(
                    args("max_symbols", "max_symbol_size"),
                    "Factory constructor.\n\n"
                    "\t:param max_symbols: "
                    "The maximum symbols the coders can expect.\n"
                    "\t:param max_symbol_size: "
                    "The maximum size of a symbol in bytes.\n"))
            .def("build", &factory_type::build,
                 "Build the actual coder.\n\n"
                 "\t:returns: An instantiation of a coder.\n")
            .def("set_symbols", &factory_type::set_symbols, arg("symbols"),
                 "Set the number of symbols.\n\n"
                 "\t:param symbols: The number of symbols.\n"
                )
            .def("symbols", &factory_type::symbols,
                 "Return the number of symbols in a block.\n\n"
                 "\t:returns: The number of symbols in a block.\n"
                )
            .def("max_symbols", &factory_type::max_symbols,
                 "Return the maximum number of symbols in a block.\n\n"
                 "\t:returns: The maximum number of symbols in a block.\n"
                )
            .def("set_symbol_size", &factory_type::set_symbol_size,
                 arg("symbol_size"),
                 "Set the symbol size.\n\n"
                 "\t:param symbols_size: The symbol size.\n"
                )
            .def("symbol_size", &factory_type::symbol_size,
                 "Return the symbol size in bytes.\n\n"
                 "\t:returns: The symbol size in bytes.\n"
                )
            .def("max_symbol_size", &factory_type::max_symbol_size,
                 "Return the maximum symbol size in bytes.\n\n"
                 "\t:returns: The maximum symbol size in bytes.\n"
                )
            .def("max_payload_size", &factory_type::max_payload_size,
                 "Return the maximum required payload buffer size in bytes.\n\n"
                 "\t:returns: The maximum required payload buffer size in bytes.\n"
                );

        std::string max_block_size_desc;
        if (kodo_core::has_is_complete<stack_type>::value)
        {
            max_block_size_desc =
                "Return the maximum amount of data decoded in bytes.\n\n"
                "This is calculated by multiplying the maximum number of symbols "
                "decoded by the maximum size of a symbol.\n\n"
                "\t:returns: The maximum amount of data decoded in bytes\n";
        }
        else
        {
            max_block_size_desc =
                "Return the maximum amount of data encoded in bytes.\n\n"
                "This is calculated by multiplying the maximum number of symbols "
                "encoded by the maximum size of a symbol.\n\n"
                "\t:returns: The maximum amount of data encoded in bytes\n";
        }

        factory_class.def("max_block_size", &factory_type::max_block_size,
                          max_block_size_desc.c_str()
                         );

        (extra_factory_methods<Coder>(factory_class));

        // Enable boost to map from the c++ pointer type to the python coder
        // type. E.g., from std::shared_ptr<Codec> to python [Codec]Encoder.
        boost::python::register_ptr_to_python<typename factory_type::pointer>();
    }
}
