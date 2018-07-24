// Copyright Steinwurf ApS 2015.
// Distributed under the "STEINWURF EVALUATION LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#if !defined(KODO_PYTHON_DISABLE_NOCODE)

// boost/python.hpp should be the first include in every .cpp file to work
// around this bug: https://bugs.python.org/issue10910
#include <boost/python.hpp>

#include <kodo_core/nocode/carousel_decoder.hpp>
#include <kodo_core/nocode/carousel_encoder.hpp>

#include "create_helpers.hpp"

namespace kodo_python
{
void create_carousel_stacks()
{
    using namespace kodo_core::nocode;

    create_factory_and_encoder<carousel_encoder>("NoCodeEncoder");
    create_factory_and_decoder<carousel_decoder>("NoCodeDecoder");
}
}

#endif
