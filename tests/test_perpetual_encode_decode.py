#!/usr/bin/env python
# encoding: utf-8

"""Tests perpetual encoding and decoding"""

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
import unittest
import kodo


class TestPerpetualEncodeDecode(unittest.TestCase):
    def test_encode_decode(self):
        widths = [
            kodo.perpetual.Width._8,
            kodo.perpetual.Width._16,
            kodo.perpetual.Width._32,
            kodo.perpetual.Width._64,
        ]
        for width in widths:
            with self.subTest(width):
                self.encode_decode(width)

    def encode_decode(self, width):

        symbol_bytes = 1400

        block_bytes = 1000000

        encoder = kodo.perpetual.Encoder(width)
        decoder = kodo.perpetual.Decoder(width)

        encoder.configure(block_bytes, symbol_bytes)
        decoder.configure(block_bytes, symbol_bytes)

        self.assertEqual(encoder.symbol_bytes, symbol_bytes)
        self.assertEqual(encoder.block_bytes, block_bytes)

        self.assertEqual(decoder.symbol_bytes, symbol_bytes)
        self.assertEqual(decoder.block_bytes, block_bytes)

        generator = kodo.perpetual.generator.RandomUniform(width)
        offset_generator = kodo.perpetual.offset.RandomUniform()

        offset_generator.configure(encoder.symbols)

        symbol = bytearray(encoder.symbol_bytes)

        data_in = bytearray(os.urandom(encoder.block_bytes))
        encoder.set_symbols_storage(data_in)

        data_out = bytearray(decoder.block_bytes)
        decoder.set_symbols_storage(data_out)
        seed = 0

        while not decoder.is_complete():

            offset = offset_generator.offset()

            coefficients = generator.generate(seed)

            encoder.encode_symbol(symbol, coefficients, offset)

            decoder.decode_symbol(symbol, coefficients, offset)

            seed += 1

        self.assertEqual(data_in, data_out)


if __name__ == "__main__":
    unittest.main()
