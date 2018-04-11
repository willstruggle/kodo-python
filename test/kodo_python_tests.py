#! /usr/bin/env python
# encoding: utf-8

# Copyright Steinwurf ApS 2015.
# Distributed under the "STEINWURF EVALUATION LICENSE 1.0".
# See accompanying file LICENSE.rst or
# http://www.steinwurf.com/licensing

import os
import unittest

import kodo

# The test sets are only enabled for the codecs that were actually compiled
# (some repositories might be unavailable for certain users)
test_sets = []

# FullVector
if hasattr(kodo, 'FullVectorEncoderFactoryBinary'):
    test_sets.append((kodo.FullVectorEncoderFactoryBinary,
                      kodo.FullVectorDecoderFactoryBinary))
if hasattr(kodo, 'FullVectorEncoderFactoryBinary4'):
    test_sets.append((kodo.FullVectorEncoderFactoryBinary4,
                      kodo.FullVectorDecoderFactoryBinary4))
if hasattr(kodo, 'FullVectorEncoderFactoryBinary8'):
    test_sets.append((kodo.FullVectorEncoderFactoryBinary8,
                      kodo.FullVectorDecoderFactoryBinary8))
if hasattr(kodo, 'FullVectorEncoderFactoryBinary16'):
    test_sets.append((kodo.FullVectorEncoderFactoryBinary16,
                      kodo.FullVectorDecoderFactoryBinary16))

# SparseFullVector
if hasattr(kodo, 'SparseFullVectorEncoderFactoryBinary'):
    test_sets.append((kodo.SparseFullVectorEncoderFactoryBinary,
                      kodo.FullVectorDecoderFactoryBinary))
if hasattr(kodo, 'SparseFullVectorEncoderFactoryBinary4'):
    test_sets.append((kodo.SparseFullVectorEncoderFactoryBinary4,
                      kodo.FullVectorDecoderFactoryBinary4))
if hasattr(kodo, 'SparseFullVectorEncoderFactoryBinary8'):
    test_sets.append((kodo.SparseFullVectorEncoderFactoryBinary8,
                      kodo.FullVectorDecoderFactoryBinary8))
if hasattr(kodo, 'SparseFullVectorEncoderFactoryBinary16'):
    test_sets.append((kodo.SparseFullVectorEncoderFactoryBinary16,
                      kodo.FullVectorDecoderFactoryBinary16))

# OnTheFly
if hasattr(kodo, 'OnTheFlyEncoderFactoryBinary'):
    test_sets.append((kodo.OnTheFlyEncoderFactoryBinary,
                      kodo.OnTheFlyDecoderFactoryBinary))
if hasattr(kodo, 'OnTheFlyEncoderFactoryBinary4'):
    test_sets.append((kodo.OnTheFlyEncoderFactoryBinary4,
                      kodo.OnTheFlyDecoderFactoryBinary4))
if hasattr(kodo, 'OnTheFlyEncoderFactoryBinary8'):
    test_sets.append((kodo.OnTheFlyEncoderFactoryBinary8,
                      kodo.OnTheFlyDecoderFactoryBinary8))
if hasattr(kodo, 'OnTheFlyEncoderFactoryBinary16'):
    test_sets.append((kodo.OnTheFlyEncoderFactoryBinary16,
                      kodo.OnTheFlyDecoderFactoryBinary16))

# SlidingWindow
if hasattr(kodo, 'SlidingWindowEncoderFactoryBinary'):
    test_sets.append((kodo.SlidingWindowEncoderFactoryBinary,
                      kodo.SlidingWindowDecoderFactoryBinary))
if hasattr(kodo, 'SlidingWindowEncoderFactoryBinary4'):
    test_sets.append((kodo.SlidingWindowEncoderFactoryBinary4,
                      kodo.SlidingWindowDecoderFactoryBinary4))
if hasattr(kodo, 'SlidingWindowEncoderFactoryBinary8'):
    test_sets.append((kodo.SlidingWindowEncoderFactoryBinary8,
                      kodo.SlidingWindowDecoderFactoryBinary8))
if hasattr(kodo, 'SlidingWindowEncoderFactoryBinary16'):
    test_sets.append((kodo.SlidingWindowEncoderFactoryBinary16,
                      kodo.SlidingWindowDecoderFactoryBinary16))

# Perpetual
if hasattr(kodo, 'PerpetualEncoderFactoryBinary'):
    test_sets.append((kodo.PerpetualEncoderFactoryBinary,
                      kodo.PerpetualDecoderFactoryBinary))
if hasattr(kodo, 'PerpetualEncoderFactoryBinary4'):
    test_sets.append((kodo.PerpetualEncoderFactoryBinary4,
                      kodo.PerpetualDecoderFactoryBinary4))
if hasattr(kodo, 'PerpetualEncoderFactoryBinary8'):
    test_sets.append((kodo.PerpetualEncoderFactoryBinary8,
                      kodo.PerpetualDecoderFactoryBinary8))
if hasattr(kodo, 'PerpetualEncoderFactoryBinary16'):
    test_sets.append((kodo.PerpetualEncoderFactoryBinary16,
                      kodo.PerpetualDecoderFactoryBinary16))

# Carousel
if hasattr(kodo, 'NoCodeEncoderFactory'):
    test_sets.append((kodo.NoCodeEncoderFactory,
                      kodo.NoCodeDecoderFactory))

# Fulcrum
if hasattr(kodo, 'FulcrumEncoderFactoryBinary4'):
    test_sets.append((kodo.FulcrumEncoderFactoryBinary4,
                      kodo.FulcrumDecoderFactoryBinary4))
if hasattr(kodo, 'FulcrumEncoderFactoryBinary8'):
    test_sets.append((kodo.FulcrumEncoderFactoryBinary8,
                      kodo.FulcrumDecoderFactoryBinary8))
if hasattr(kodo, 'FulcrumEncoderFactoryBinary16'):
    test_sets.append((kodo.FulcrumEncoderFactoryBinary16,
                      kodo.FulcrumDecoderFactoryBinary16))


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
