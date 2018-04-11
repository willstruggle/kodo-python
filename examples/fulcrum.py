#! /usr/bin/env python
# encoding: utf-8

# Copyright Steinwurf ApS 2015.
# Distributed under the "STEINWURF EVALUATION LICENSE 1.0".
# See accompanying file LICENSE.rst or
# http://www.steinwurf.com/licensing

import os
import sys
import kodo


def main():
    """
    Fulcrum example.

    This Example shows how to use the additional settings and parameters
    supported by the fulcrum code.
    """
    if not hasattr(kodo, 'FulcrumEncoderFactoryBinary8'):
        print("The fulcrum codec is not available")
        return

    # Set the number of symbols (i.e. the generation size in RLNC terminology)
    # and the size of a symbol in bytes
    symbols = 24
    symbol_size = 160

    # Create encoder/decoder factory used to build actual encoders/decoders
    encoder_factory = kodo.FulcrumEncoderFactoryBinary8(symbols, symbol_size)
    decoder_factory = kodo.FulcrumDecoderFactoryBinary8(symbols, symbol_size)

    # The expansion factor denotes the number of additional symbols created by
    # the outer code.
    print(
        "The default values for the fulcrum factories are the following:\n"
        "\tSymbols: {}\n"
        "\tMax expansion: {}\n"
        "\tExpansion: {}\n"
        "\tMax inner symbols: {}".format(
            encoder_factory.max_symbols(),
            encoder_factory.max_expansion(),
            encoder_factory.expansion(),
            encoder_factory.max_inner_symbols()))

    new_expansion = encoder_factory.max_expansion() - 1

    print("Let's set the encoder factory's expansion to {}.".format(
        new_expansion))

    encoder_factory.set_expansion(new_expansion)

    # now let's build the coders
    encoder = encoder_factory.build()
    decoder = decoder_factory.build()

    print(
        "The created coders now have the following settings.\n"
        "Encoder:\n"
        "\tSymbols: {}\n"
        "\tExpansion: {}\n"
        "\tInner symbols: {}\n"
        "Decoder:\n"
        "\tSymbols: {}\n"
        "\tExpansion: {}\n"
        "\tInner symbols: {}".format(
            encoder.symbols(),
            encoder.expansion(),
            encoder.inner_symbols(),
            decoder.symbols(),
            decoder.expansion(),
            decoder.inner_symbols()))

    # Create some data to encode. In this case we make a buffer
    # with the same size as the encoder's block size (the max.
    # amount a single encoder can encode)
    # Just for fun - fill the input data with random data
    data_in = os.urandom(encoder.block_size())

    # Assign the data buffer to the encoder so that we can
    # produce encoded symbols
    encoder.set_const_symbols(data_in)

    while not decoder.is_complete():
        # Encode a packet into the payload buffer
        payload = encoder.write_payload()

        # Pass that packet to the decoder
        decoder.read_payload(payload)

    # The decoder is complete, now copy the symbols from the decoder
    data_out = decoder.copy_from_symbols()

    # Check we properly decoded the data
    if data_out == data_in:
        print("Data decoded correctly")
    else:
        print("Unexpected failure to decode please file a bug report :)")
        sys.exit(1)

if __name__ == "__main__":
    main()
