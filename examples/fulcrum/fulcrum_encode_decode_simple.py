#!/usr/bin/env python
# encoding: utf-8

# License for Commercial Usage
# Distributed under the "KODO EVALUATION LICENSE 1.3"
# Licensees holding a valid commercial license may use this project in
# accordance with the standard license agreement terms provided with the
# Software (see accompanying file LICENSE.rst or
# https://www.steinwurf.com/license), unless otherwise different terms and
# conditions are agreed in writing between Licensee and Steinwurf ApS in which
# case the license will be regulated by that separate written agreement.
# License for Non-Commercial Usage
# Distributed under the "KODO RESEARCH LICENSE 1.2"
# Licensees holding a valid research license may use this project in accordance
# with the license agreement terms provided with the Software
# See accompanying file LICENSE.rst or https://www.steinwurf.com/license

import os
import random

import kodo


def main():
    """
    Simple example showing how to encode and decode a block of memory using
    the fulcrum code.
    """
    field = kodo.FiniteField.binary

    symbol_bytes = 1400
    symbols = 150
    expansion = 15

    encoder = kodo.fulcrum.Encoder(field)
    encoder.configure(symbols, symbol_bytes, expansion)

    decoder = kodo.fulcrum.Decoder(field)
    decoder.configure(symbols, symbol_bytes, expansion)

    generator = kodo.fulcrum.generator.RandomUniform()
    generator.configure(encoder.symbols, encoder.expansion)

    generator.set_seed(0)

    symbol = bytearray(encoder.symbol_bytes)

    coefficients = bytearray(generator.max_coefficients_bytes)

    data_in = bytearray(os.urandom(encoder.block_bytes))
    encoder.set_symbols_storage(data_in)

    data_out = bytearray(decoder.block_bytes)
    decoder.set_symbols_storage(data_out)

    systematic_index = 0

    loss_probability = 20

    while not decoder.is_complete():

        if encoder.symbols > systematic_index:

            index = systematic_index
            systematic_index += 1
            encoder.encode_systematic_symbol(symbol, index)

            if random.randint(0, 100) < loss_probability:
                print(" - lost")
                continue

            else:

                decoder.decode_systematic_symbol(symbol, index)
                print(f" - decoded, rank now {decoder.inner_rank}")

        else:

            generator.generate(coefficients)
            encoder.encode_symbol(symbol, coefficients)

            if random.randint(0, 100) < loss_probability:
                print(" - lost")
                continue

            else:

                decoder.decode_symbol(symbol, coefficients)
                print(f" - decoded, rank now {decoder.inner_rank}")

    if data_in == data_out:
        print("Data decoded correctly!")
        return 0

    else:
        print("Unexpected failure to decode")
        return 1


if __name__ == "__main__":
    main()
