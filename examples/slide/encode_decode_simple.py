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
import sys
import random

import kodo


def main():

    max_symbol_bytes = 1400
    encoder_capacity = 20
    decoder_capacity = 40

    field = kodo.FiniteField.binary8

    encoder = kodo.slide.Encoder(field)
    encoder.configure(max_symbol_bytes)

    decoder = kodo.slide.Decoder(field)
    decoder.configure(max_symbol_bytes)

    generator = kodo.slide.generator.RandomUniform(field)

    rate = kodo.slide.RateController(n=10, k=4)

    # Keeping track of decoded symbols
    decoded = []

    decoder.on_symbol_decoded(lambda index: decoded.append(index))

    # Lose packets with 10% probability
    loss_probability = 10

    index = 0

    for _ in range(1000):
        # Manage the encoder's window
        repair = rate.send_repair()
        if repair:
            # Encode a symbol
            window = encoder.stream_range

            # Choose a seed for this encoding
            seed = random.randint(0, 100000)
            generator.set_seed(seed)
            coefficients = generator.generate(window)
            symbol = encoder.encode_symbol(window, coefficients)

            print(f"coded symbol {window}")
        else:
            # If window is full - pop a symbol before pushing a new one
            if encoder.stream_symbols == encoder_capacity:
                encoder.pop_symbol()

            # Create a new source symbol and add it to the encoder
            encoder.push_symbol(bytearray(os.urandom(encoder.max_symbol_bytes)))
            symbol = encoder.encode_systematic_symbol(index)
            print(f"systematic symbol {index}", end="")

        # Update loop state
        rate.advance()

        # Drop packet based on loss probability
        if random.randint(0, 100) < loss_probability:
            print(" - lost")
            continue
        print(" - received")

        # Move the decoders's window / stream if needed
        #
        # If the encoder includes symbols in its window that the decoder
        # does not have. We need to update the state of the decoder.
        # In the following we will go through two cases:
        #
        # Case 1: The decoder can move its stream front by adding more
        #         symbols. This is possible if it has not reached the
        #         maximum capacity yet.
        #
        # Case 2: If the decoder is a maximum capacity it needs to
        #         slide its window/stream - dropping symbols that are
        #         now too "old".

        # Check if the decoder's stream is behind the encoder
        while decoder.stream_upper_bound < encoder.stream_upper_bound:
            if decoder.stream_symbols >= decoder_capacity:
                # Remove the "oldest" symbol
                decoder.pop_symbol()
            decoder.push_symbol()

        if repair:

            # Set the seed and window on the generator
            generator.set_seed(seed)
            coefficients = generator.generate(window)

            decoder.decode_symbol(symbol, window, coefficients)
        else:
            decoder.decode_systematic_symbol(symbol, index)
            index += 1

        # New symbols may now be decoded.
        for i in range(decoder.stream_symbols):

            index = i + decoder.stream_lower_bound

            if not decoder.is_symbol_decoded(index):
                # This symbol has not yet been decoded
                continue
            if index in decoded:
                continue
            decoded.append(index)
            if decoder.in_stream(index) and encoder.in_stream(index):
                # If symbol is available on both the encoder and decoder,
                # check that they do indeed contain the same data.
                data_out = decoder.symbol_data(index)
                data_in = encoder.symbol_data(index)

                if data_in == data_out:
                    print(f" decoded {index}!")
                else:
                    print(f" decoding failed {index}!")
                    sys.exit(1)


if __name__ == "__main__":
    main()
