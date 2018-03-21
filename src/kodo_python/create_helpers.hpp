// Copyright Steinwurf ApS 2015.
// Distributed under the "STEINWURF EVALUATION LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#pragma once

#include <string>

#include <fifi/binary.hpp>
#include <fifi/binary4.hpp>
#include <fifi/binary8.hpp>
#include <fifi/binary16.hpp>

#include <kodo_core/enable_trace.hpp>

#include "encoder.hpp"
#include "decoder.hpp"
#include "factory.hpp"
#include "resolve_field_name.hpp"

namespace kodo_python
{
template<template<class, class> class Coder, class Field>
void create_factory_and_encoder(const std::string& stack)
{
    // First create the factory type
    factory<Coder, Field, meta::typelist<kodo_core::enable_trace>>(stack);
    // Then create the corresponding encoder type
    encoder<Coder, Field, meta::typelist<kodo_core::enable_trace>>(stack);
}

template<template<class, class> class Coder, class Field>
void create_factory_and_decoder(const std::string& stack)
{
    // First create the factory type
    factory<Coder, Field, meta::typelist<kodo_core::enable_trace>>(stack);
    // Then create the corresponding decoder type
    decoder<Coder, Field, meta::typelist<kodo_core::enable_trace>>(stack);
}

template<template<class, class> class Coder>
void create_encoder(const std::string& stack)
{
    create_factory_and_encoder<Coder, fifi::binary>(stack);
    create_factory_and_encoder<Coder, fifi::binary4>(stack);
    create_factory_and_encoder<Coder, fifi::binary8>(stack);
    create_factory_and_encoder<Coder, fifi::binary16>(stack);
}

template<template<class, class> class Coder>
void create_decoder(const std::string& stack)
{
    create_factory_and_decoder<Coder, fifi::binary>(stack);
    create_factory_and_decoder<Coder, fifi::binary4>(stack);
    create_factory_and_decoder<Coder, fifi::binary8>(stack);
    create_factory_and_decoder<Coder, fifi::binary16>(stack);
}
}
