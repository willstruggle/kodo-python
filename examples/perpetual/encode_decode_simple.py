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

import kodo


def main():
    """
    Simple example showing how to encode and decode a block of memory
    using perpetual coders
    """

    # Pick the width to use for the encoding and decoding.
    width = kodo.perpetual.Width._8

    # Create an encoder and decoder.
    encoder = kodo.perpetual.Encoder(width)
    decoder = kodo.perpetual.Decoder(width)

    # Pick the size of each symbol in bytes
    symbol_bytes = 1400

    # Pick the number of bytes to encode/decode.
    block_bytes = 1000000

    # Configure the coders, the encoder and decoder must be configured
    # identically to be compatible.
    encoder.configure(block_bytes, symbol_bytes)
    decoder.configure(block_bytes, symbol_bytes)

    # Create generator and offset generator.
    generator = kodo.perpetual.generator.RandomUniform(width)
    offset_generator = kodo.perpetual.offset.RandomUniform()

    # Configure offset generator.
    # The generator must similarly be configured based on the encoder/decoder.
    offset_generator.configure(encoder.symbols)

    # Allocate some storage for a symbol.
    symbol = bytearray(encoder.symbol_bytes)

    # Allocate some data to encode. In this case we make a buffer
    # with the same size as the encoder's block size (the max.
    # amount a single encoder can encode)
    # Just for fun - fill data_in with random data
    data_in = bytearray(os.urandom(encoder.block_bytes))

    # Assign the data buffer to the encoder so that we may start
    # to produce encoded symbols from it
    encoder.set_symbols_storage(data_in)

    # Define a data buffer where the symbols should be decoded
    data_out = bytearray(decoder.block_bytes)
    decoder.set_symbols_storage(data_out)

    seed = 0

    while not decoder.is_complete():
        # Generate the offset
        offset = offset_generator.offset()

        # Generate the coefficients into the symbol buffer
        coefficients = generator.generate(seed)

        # Encode a symbol into the symbol buffer
        encoder.encode_symbol(symbol, coefficients, offset)

        # Note; if this were two septate applications communicating over a
        # network the encoder and decoder would need to exchange some
        # information.
        # Besides the obvious symbol data, the encoder would also need to
        # share the coefficients and offset
        # (the state for the decoder to generate the information on it's own).

        # Pass the symbol, coefficients, and offset to the decoder
        decoder.decode_symbol(symbol, coefficients, offset)

        seed += 1

    # Check if we properly decoded the data
    if data_in != data_out:
        print("Something went wrong. Decoding was unsuccessful.")

    else:
        print("Decoding was successful. Yay!")


if __name__ == "__main__":
    main()
