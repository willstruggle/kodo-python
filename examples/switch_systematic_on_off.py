#! /usr/bin/env python
# encoding: utf-8

# Copyright Steinwurf ApS 2011-2013.
# Distributed under the "STEINWURF EVALUATION LICENSE 1.0".
# See accompanying file LICENSE.rst or
# http:#www.steinwurf.com/licensing

import os
import random
import sys

import kodo


def main():
    """
    Switch systematic off example.

    This example shows how to enable or disable systematic coding for
    coding stacks that support it.
    Systematic coding is used to reduce the amount of work done by an
    encoder and a decoder. This is achieved by initially sending all
    symbols which has not previously been sent uncoded. Kodo allows this
    feature to be optionally turn of or off.
    """
    # Set the number of symbols (i.e. the generation size in RLNC
    # terminology) and the size of a symbol in bytes
    symbols = 16
    symbol_size = 160

    # In the following we will make an encoder/decoder factory.
    # The factories are used to build actual encoders/decoders
    encoder_factory = kodo.FullVectorEncoderFactoryBinary(
        max_symbols=symbols,
        max_symbol_size=symbol_size)

    encoder = encoder_factory.build()

    decoder_factory = kodo.FullVectorDecoderFactoryBinary(
        max_symbols=symbols,
        max_symbol_size=symbol_size)

    decoder = decoder_factory.build()

    # Create some data to encode. In this case we make a buffer
    # with the same size as the encoder's block size (the max.
    # amount a single encoder can encode)
    # Just for fun - fill the input data with random data
    data_in = os.urandom(encoder.block_size())

    # Assign the data buffer to the encoder so that we may start
    # to produce encoded symbols from it
    encoder.set_const_symbols(data_in)

    print("Starting encoding / decoding")

    while not decoder.is_complete():

        # If the chosen codec stack supports systematic coding
        if 'is_systematic_on' in dir(encoder):

            # With 50% probability toggle systematic
            if random.choice([True, False]):

                if encoder.is_systematic_on():
                    print("Turning systematic OFF")
                    encoder.set_systematic_off()
                else:
                    print("Turning systematic ON")
                    encoder.set_systematic_on()

        # Encode a packet into the payload buffer
        packet = encoder.write_payload()

        if random.choice([True, False]):
            print("Drop packet")
            continue

        # Pass that packet to the decoder
        decoder.read_payload(packet)

        print("Rank of decoder {}".format(decoder.rank()))

        # Symbols that were received in the systematic phase correspond
        # to the original source symbols and are therefore marked as
        # decoded
        print("Symbols decoded {}".format(decoder.symbols_uncoded()))

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
