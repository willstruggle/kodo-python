#! /usr/bin/env python
# encoding: utf-8

# Copyright Steinwurf ApS 2015.
# Distributed under the "STEINWURF EVALUATION LICENSE 1.0".
# See accompanying file LICENSE.rst or
# http://www.steinwurf.com/licensing

import os
import unittest

import kodo

test_sets = [
    # FullVector
    (kodo.FullVectorEncoderFactoryBinary,
     kodo.FullVectorDecoderFactoryBinary),
    (kodo.FullVectorEncoderFactoryBinary4,
     kodo.FullVectorDecoderFactoryBinary4),
    (kodo.FullVectorEncoderFactoryBinary8,
     kodo.FullVectorDecoderFactoryBinary8),
    (kodo.FullVectorEncoderFactoryBinary16,
     kodo.FullVectorDecoderFactoryBinary16),

    # SparseFullVector
    (kodo.SparseFullVectorEncoderFactoryBinary,
     kodo.FullVectorDecoderFactoryBinary),
    (kodo.SparseFullVectorEncoderFactoryBinary4,
     kodo.FullVectorDecoderFactoryBinary4),
    (kodo.SparseFullVectorEncoderFactoryBinary8,
     kodo.FullVectorDecoderFactoryBinary8),
    (kodo.SparseFullVectorEncoderFactoryBinary16,
     kodo.FullVectorDecoderFactoryBinary16),

    # OnTheFly
    (kodo.OnTheFlyEncoderFactoryBinary,
     kodo.OnTheFlyDecoderFactoryBinary),
    (kodo.OnTheFlyEncoderFactoryBinary4,
     kodo.OnTheFlyDecoderFactoryBinary4),
    (kodo.OnTheFlyEncoderFactoryBinary8,
     kodo.OnTheFlyDecoderFactoryBinary8),
    (kodo.OnTheFlyEncoderFactoryBinary16,
     kodo.OnTheFlyDecoderFactoryBinary16),

    # SlidingWindow
    (kodo.SlidingWindowEncoderFactoryBinary,
     kodo.SlidingWindowDecoderFactoryBinary),
    (kodo.SlidingWindowEncoderFactoryBinary4,
     kodo.SlidingWindowDecoderFactoryBinary4),
    (kodo.SlidingWindowEncoderFactoryBinary8,
     kodo.SlidingWindowDecoderFactoryBinary8),
    (kodo.SlidingWindowEncoderFactoryBinary16,
     kodo.SlidingWindowDecoderFactoryBinary16),

    # Perpetual
    (kodo.PerpetualEncoderFactoryBinary,
     kodo.PerpetualDecoderFactoryBinary),
    (kodo.PerpetualEncoderFactoryBinary4,
     kodo.PerpetualDecoderFactoryBinary4),
    (kodo.PerpetualEncoderFactoryBinary8,
     kodo.PerpetualDecoderFactoryBinary8),
    (kodo.PerpetualEncoderFactoryBinary16,
     kodo.PerpetualDecoderFactoryBinary16),

    # Carousel
    (kodo.NoCodeEncoderFactory,
     kodo.NoCodeDecoderFactory),

    # Fulcrum
    (kodo.FulcrumEncoderFactoryBinary4,
     kodo.FulcrumDecoderFactoryBinary4),
    (kodo.FulcrumEncoderFactoryBinary8,
     kodo.FulcrumDecoderFactoryBinary8),
    (kodo.FulcrumEncoderFactoryBinary16,
     kodo.FulcrumDecoderFactoryBinary16),
]


class TestVersion(unittest.TestCase):

    def test_version(self):
        versions = kodo.__version__.split('\n')
        for version in versions:
            # Make sure that a version number is available for all
            # dependencies.
            self.assertNotEqual(
                version.split(':')[1].strip(), '', msg=version.strip())


class TestEncodeDecode(unittest.TestCase):

    def test_all(self):
        for test_set in test_sets:
            self.encode_decode_simple(*test_set)

    def encode_decode_simple(self, EncoderFactory, DecoderFactory):
        # Set the number of symbols (i.e. the generation size in RLNC
        # terminology) and the size of a symbol in bytes
        symbols = 8
        symbol_size = 160

        # In the following we will make an encoder/decoder factory.
        # The factories are used to build actual encoders/decoders
        encoder_factory = EncoderFactory(symbols, symbol_size)
        encoder = encoder_factory.build()

        decoder_factory = DecoderFactory(symbols, symbol_size)
        decoder = decoder_factory.build()

        # Create some data to encode. In this case we make a buffer
        # with the same size as the encoder's block size (the max.
        # amount a single encoder can encode)
        # Just for fun - fill the input data with random data
        data_in = bytearray(os.urandom(encoder.block_size()))
        data_in = bytes(data_in)

        # Assign the data buffer to the encoder so that we can
        # produce encoded symbols
        encoder.set_const_symbols(data_in)

        while not decoder.is_complete():
            # Generate an encoded packet
            packet = encoder.write_payload()
            # Decode the encoded packet
            decoder.read_payload(packet)

        # The decoder is complete, now copy the symbols from the decoder
        data_out = decoder.copy_from_symbols()

        # Check if we properly decoded the data
        self.assertEqual(data_out, data_in)


def main():
    unittest.main()

if __name__ == "__main__":
    main()
