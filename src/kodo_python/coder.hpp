// Copyright Steinwurf ApS 2015.
// Distributed under the "STEINWURF EVALUATION LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#pragma once

#include <string>

#include <boost/python.hpp>

#include <kodo_core/has_rank.hpp>

#include <kodo_core/has_is_symbol_pivot.hpp>
#include <kodo_core/has_set_zone_prefix.hpp>
#include <kodo_core/has_set_trace_stdout.hpp>
#include <kodo_core/has_set_trace_callback.hpp>

namespace kodo_python
{
template<bool HasIsSymbolPivot>
struct is_symbol_pivot_method
{
    template<class CoderClass>
    is_symbol_pivot_method(CoderClass& coder_class)
    {
        (void) coder_class;
    }
};

template<>
struct is_symbol_pivot_method<true>
{
    template<class CoderClass>
    is_symbol_pivot_method(CoderClass& coder_class)
    {
        coder_class
        .def("is_symbol_pivot", &CoderClass::wrapped_type::is_symbol_pivot,
             boost::python::arg("symbol_index"),
             "Check if a certain symbol is a pivot.\n\n"
             "A symbol is pivot if it is available to either the encoder or "
             "decoder. A coefficient generator may use this information "
             "when generating coding coefficients.\n\n"
             "\t:param symbol_index: The index of the symbol.\n"
             "\t:returns: True if the symbol is available.\n"
            );
    }
};

template<bool HasRank>
struct rank_method
{
    template<class CoderClass>
    rank_method(CoderClass& coder_class)
    {
        (void) coder_class;
    }
};

template<>
struct rank_method<true>
{
    template<class CoderClass>
    rank_method(CoderClass& coder_class)
    {
        coder_class
        .def("rank", &CoderClass::wrapped_type::rank,
             "Return the rank.\n\n"
             "The rank of a decoder states how many symbols have been "
             "decoded or partially decoded. The rank of an encoder states "
             "how many symbols are available for encoding.\n\n"
             "\t:returns: The rank.\n"
            );
    }
};

template<class Coder>
void set_trace_callback(Coder& coder, PyObject* function)
{
    auto callback = [function](
        const std::string& zone, const std::string& message)
    {
        boost::python::call<void>(function, zone, message);
    };

    coder.set_trace_callback(callback);
}

template<bool HasSetTraceCallback>
struct set_trace_callback_method
{
    template<class CoderClass>
    set_trace_callback_method(CoderClass& coder_class)
    {
        (void) coder_class;
    }
};

template<>
struct set_trace_callback_method<true>
{
    template<class CoderClass>
    set_trace_callback_method(CoderClass& coder_class)
    {
        coder_class
        .def("set_trace_callback",
             &set_trace_callback<typename CoderClass::wrapped_type>,
             boost::python::arg("callback"),
             "Write the trace information to a callback.\n\n"
             "\t:param callback: The callback which is called with the zone "
             "and message.");
    }
};

template<bool HasSetTraceStdout>
struct set_trace_stdout_method
{
    template<class CoderClass>
    set_trace_stdout_method(CoderClass& coder_class)
    {
        (void) coder_class;
    }
};

template<>
struct set_trace_stdout_method<true>
{
    template<class CoderClass>
    set_trace_stdout_method(CoderClass& coder_class)
    {
        coder_class
        .def("set_trace_stdout",
             &CoderClass::wrapped_type::set_trace_stdout,
             "Trace debug info to stdout.\n");
    }
};

template<bool HasSetZonePrefix>
struct set_zone_prefix_method
{
    template<class CoderClass>
    set_zone_prefix_method(CoderClass& coder_class)
    {
        (void) coder_class;
    }
};

template<>
struct set_zone_prefix_method<true>
{
    template<class CoderClass>
    set_zone_prefix_method(CoderClass& coder_class)
    {
        coder_class
        .def("set_zone_prefix", &CoderClass::wrapped_type::set_zone_prefix,
             boost::python::arg("zone_prefix"),
             "Sets a zone prefix for the tracing output.\n\n"
             "\t:param zone_prefix: The zone prefix to append to all "
             "tracing zones.");
    }
};

template<
    template<class, class> class Coder,
    class Field, class TraceTag
>
auto coder(const std::string& name) ->
    boost::python::class_<Coder<Field, TraceTag>, boost::noncopyable>
{
    using namespace boost::python;

    typedef Coder<Field, TraceTag> coder_type;
    auto coder_class = class_<coder_type, boost::noncopyable>(
        name.c_str(), "An (en/de)coder", no_init)
        .def("payload_size", &coder_type::payload_size,
             "Return the required payload buffer size in bytes.\n\n"
             "\t:returns: The required payload buffer size in bytes.\n"
            )
        .def("block_size", &coder_type::block_size,
             "Return the block size.\n\n"
             "\t:returns: The block size i.e. the total size in bytes that "
             "this coder operates on.\n"
            )
        .def("symbol_size", &coder_type::symbol_size,
             "Return the symbol size of a symbol in bytes.\n\n"
             "\t:returns: The symbol size of a symbol in bytes.\n"
            )
        .def("symbols", &coder_type::symbols,
             "Return the number of symbols in this block coder.\n\n"
             "\t:returns: The number of symbols in this block coder.\n"
            );

    (is_symbol_pivot_method<
     kodo_core::has_is_symbol_pivot<coder_type>::value>(coder_class));

    (rank_method<kodo_core::has_rank<coder_type>::value>(coder_class));

    // Trace related
    (set_zone_prefix_method<
     kodo_core::has_set_zone_prefix<coder_type>::value>(coder_class));
    (set_trace_stdout_method<
     kodo_core::has_set_trace_stdout<coder_type>::value>(coder_class));
    (set_trace_callback_method<
     kodo_core::has_set_trace_callback<coder_type>::value>(coder_class));

    return coder_class;
}
}
