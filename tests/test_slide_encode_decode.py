#!/usr/bin/env python
# encoding: utf-8

"""Tests sliding window encoding and decoding"""

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


class TestSlideEncodeDecode(unittest.TestCase):
    def test_slide_encode_decode_simple(self):

        random_uniform_fields = [
            kodo.FiniteField.binary,
            kodo.FiniteField.binary4,
            kodo.FiniteField.binary8,
            kodo.FiniteField.binary16,
        ]

        for field in random_uniform_fields:
            with self.subTest(field):
                self.slide_encode_decode_simple(field)

    def slide_encode_decode_simple(self, field):

        max_symbol_bytes = 1400
        window_symbols = 20
        decoder_capacity = 40

        encoder = kodo.slide.Encoder(field)
        self.assertEqual(field, encoder.field)

        encoder.configure(max_symbol_bytes)
        self.assertEqual(max_symbol_bytes, encoder.max_symbol_bytes)

        decoder = kodo.slide.Decoder(field)
        self.assertEqual(field, decoder.field)

        decoder.configure(max_symbol_bytes)
        self.assertEqual(max_symbol_bytes, decoder.max_symbol_bytes)

        rate = kodo.slide.RateController(10, 1)

        generator = kodo.slide.generator.RandomUniform(field)
        self.assertEqual(field, generator.field)

        max_iterations = 1000
        iterations = 0

        decoded = []

        def on_symbol_decoded(index):
            decoded.append(index)

        decoder.on_symbol_decoded(on_symbol_decoded)
        loss_probability = 2

        while iterations < max_iterations:

            repair = rate.send_repair()
            if not repair:
                if encoder.stream_symbols == window_symbols:
                    encoder.pop_symbol()
                encoder.push_symbol(bytearray(os.urandom(encoder.max_symbol_bytes)))

            window = encoder.stream_range

            generator.set_seed(iterations)

            coefficients = generator.generate(window)

            coded_symbol = encoder.encode_symbol(window, coefficients)

            rate.advance()

            if random.randint(0, 100) < loss_probability:
                continue

            while decoder.stream_upper_bound < encoder.stream_upper_bound:
                if decoder.stream_symbols == decoder_capacity:
                    decoder.pop_symbol()
                decoder.push_symbol()

            generator.set_seed(iterations)
            coefficients = generator.generate(window)

            decoded_symbol = decoder.decode_symbol(coded_symbol, window, coefficients)

            iterations += 1


if __name__ == "__main__":
    unittest.main()
