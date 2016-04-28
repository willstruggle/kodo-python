#! /usr/bin/env python
# encoding: utf-8

# Copyright Steinwurf ApS 2015.
# Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
# See accompanying file LICENSE.rst or
# http://www.steinwurf.com/licensing

from __future__ import print_function
from __future__ import division

import kodo
import kodo_helpers

import os
import random
import sys
import time


def main():

    # Setup canvas and viewer
    size = 512
    canvas = kodo_helpers.CanvasScreenEngine(size * 2, size)

    encoder_viewer = kodo_helpers.EncodeStateViewer(
        size=size,
        canvas=canvas)

    decoder_viewer = kodo_helpers.DecodeStateViewer(
        size=size,
        canvas=canvas,
        canvas_position=(size, 0))

    canvas.start()
    try:
        # Set the number of symbols (i.e. the generation size in RLNC
        # terminology) and the size of a symbol in bytes
        symbols = 64
        symbol_size = 16

        # In the following we will make an encoder/decoder factory.
        # The factories are used to build actual encoders/decoders
        encoder_factory = kodo.FullVectorEncoderFactoryBinary8(
            max_symbols=symbols,
            max_symbol_size=symbol_size)
        encoder = encoder_factory.build()

        decoder_factory = kodo.FullVectorDecoderFactoryBinary8(
            max_symbols=symbols,
            max_symbol_size=symbol_size)
        decoder = decoder_factory.build()

        # Create some data to encode. In this case we make a buffer
        # with the same size as the encoder's block size (the max.
        # amount a single encoder can encode)
        # Just for fun - fill the input data with random data
        data_in = os.urandom(encoder.block_size())

        def decoder_callback(zone, msg):
            decoder_viewer.trace_callback(zone, msg)
        decoder.set_trace_callback(decoder_callback)

        def encoder_callback(zone, msg):
            encoder_viewer.trace_callback(zone, msg)
        encoder_viewer.set_symbols(encoder.symbols())
        encoder.set_trace_callback(encoder_callback)

        # Assign the data buffer to the encoder so that we may start
        # to produce encoded symbols from it
        encoder.set_const_symbols(data_in)
        while not decoder.is_complete():
            # Encode a packet into the payload buffer
            packet = encoder.write_payload()

            # Here we "simulate" a packet loss of approximately 50%
            # by dropping half of the encoded packets.
            # When running this example you will notice that the initial
            # symbols are received systematically (i.e. uncoded). After
            # sending all symbols once uncoded, the encoder will switch
            # to full coding, in which case you will see the full encoding
            # vectors being sent and received.
            if random.choice([True, False]):
                continue

            # Pass that packet to the decoder
            decoder.read_payload(packet)

        time.sleep(1)
    finally:
        # What ever happens, make sure we stop the viewer.
        canvas.stop()

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
