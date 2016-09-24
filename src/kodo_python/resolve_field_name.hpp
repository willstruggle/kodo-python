// Copyright Steinwurf ApS 2015.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#pragma once

#include <string>
#include <type_traits>

#include <fifi/binary.hpp>
#include <fifi/binary4.hpp>
#include <fifi/binary8.hpp>
#include <fifi/binary16.hpp>
#include <fifi/prime2325.hpp>

namespace kodo_python
{
class no_field
{ };

template<class Field>
std::string resolve_field_name()
{
    if (std::is_same<Field, no_field>::value)
    {
        return "";
    }
    else if (std::is_same<Field, fifi::binary>::value)
    {
        return "Binary";
    }
    else if (std::is_same<Field, fifi::binary4>::value)
    {
        return "Binary4";
    }
    else if (std::is_same<Field, fifi::binary8>::value)
    {
        return "Binary8";
    }
    else if (std::is_same<Field, fifi::binary16>::value)
    {
        return "Binary16";
    }
    else if (std::is_same<Field, fifi::prime2325>::value)
    {
        return "Prime2325";
    }
}
}
