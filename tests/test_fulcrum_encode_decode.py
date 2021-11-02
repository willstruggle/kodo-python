#!/usr/bin/env python
# encoding: utf-8

"""Tests fulcrum encode-decode"""

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
import unittest
import kodo


class TestFulcrumEncodeDecode(unittest.TestCase):
    def test_fulcrum_encode_decode_simple_random_uniform(self):
        random_uniform_fields = [
            kodo.FiniteField.binary,
            kodo.FiniteField.binary4,
            kodo.FiniteField.binary8,
            kodo.FiniteField.binary16,
        ]

        for field in random_uniform_fields:
            with self.subTest(field):
                self.fulcrum_encode_decode_simple_random_uniform(field)

    def fulcrum_encode_decode_simple_random_uniform(self, field):

        symbol_bytes = 1400
        symbols = 150
        expansion = 15

        encoder = kodo.fulcrum.Encoder(field)
        self.assertEqual(field, encoder.field)

        encoder.configure(symbols, symbol_bytes, expansion)
        self.assertEqual(symbols, encoder.symbols)
        self.assertEqual(symbol_bytes, encoder.symbol_bytes)
        self.assertEqual(expansion, encoder.expansion)

        decoder = kodo.fulcrum.Decoder(field)
        self.assertEqual(field, decoder.field)

        decoder.configure(symbols, symbol_bytes, expansion)
        self.assertEqual(symbols, decoder.symbols)
        self.assertEqual(symbol_bytes, decoder.symbol_bytes)
        self.assertEqual(expansion, decoder.expansion)

        generator = kodo.fulcrum.generator.RandomUniform()
        generator.configure(encoder.symbols, encoder.expansion)
        self.assertEqual(symbols, generator.symbols)
        generator.set_seed(0)

        symbol = bytearray(encoder.symbol_bytes)
        self.assertEqual(len(symbol), encoder.symbol_bytes)
        coefficients = bytearray(generator.max_coefficients_bytes)
        self.assertEqual(len(coefficients), generator.max_coefficients_bytes)

        data_in = bytearray(os.urandom(encoder.block_bytes))
        encoder.set_symbols_storage(data_in)

        data_out = bytearray(decoder.block_bytes)
        decoder.set_symbols_storage(data_out)

        systematic_index = 0

        loss_probability = 10

        iterations = symbols * 2
        while not decoder.is_complete():
            iterations -= 1
            self.assertNotEqual(0, iterations)

            if encoder.inner_symbols() > systematic_index:

                old_rank = decoder.rank
                index = systematic_index
                systematic_index += 1
                encoder.encode_systematic_symbol(symbol, index)

                if random.randint(0, 100) < loss_probability:

                    continue

                else:

                    decoder.decode_systematic_symbol(symbol, index)
                    self.assertNotEqual(old_rank, decoder.rank)

            else:

                old_rank = decoder.rank
                generator.generate(coefficients)
                encoder.encode_symbol(symbol, coefficients)

                if random.randint(0, 100) < loss_probability:

                    continue

                else:

                    decoder.decode_symbol(symbol, coefficients)

        self.assertEqual(data_in, data_out)
