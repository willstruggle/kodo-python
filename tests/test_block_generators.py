#!/usr/bin/env python
# encoding: utf-8

"""Tests block generators"""

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

import unittest
import kodo


class TestBlockGenerators(unittest.TestCase):
    def test_block_random_uniform_simple(self):
        fields = [
            kodo.FiniteField.binary,
            kodo.FiniteField.binary4,
            kodo.FiniteField.binary8,
            kodo.FiniteField.binary16,
        ]
        for field in fields:
            with self.subTest(field):
                self.block_random_uniform_simple(field)

    def block_random_uniform_simple(self, field):

        symbols = 40
        index = 5

        generator = kodo.block.generator.RandomUniform(field)
        generator.configure(symbols=symbols)
        self.assertEqual(symbols, generator.symbols)
        generator.set_seed(0)

        for index in range(symbols):

            coefficients = generator.generate()

        old_coefficients = coefficients
        generator.set_seed(0)
        coefficients = generator.generate_partial(generator.symbols)

    def test_block_rs_cauchy_simple(self):
        fields = [kodo.FiniteField.binary4, kodo.FiniteField.binary8]
        for field in fields:
            with self.subTest(field):
                self.block_rs_cauchy_simple(field)

    def block_rs_cauchy_simple(self, field):

        symbols = 14
        index = 5
        generator = kodo.block.generator.RSCauchy(field)
        generator.configure(symbols)
        self.assertEqual(symbols, generator.symbols)

    def test_block_parity_2d_simple(self):
        cols = [5, 10, 20, 40]
        rows = [40, 20, 10, 5]
        for row in rows:
            for col in cols:
                with self.subTest(f"{row}x{col}"):
                    self.block_parity_2d_simple(row, col)

    def block_parity_2d_simple(self, rows, cols):

        generator_both = kodo.block.generator.Parity2D()
        generator_both.configure(rows, cols)
        symbols = generator_both.symbols
        self.assertEqual(generator_both.row_redundancy_enabled(), True)
        self.assertEqual(generator_both.column_redundancy_enabled(), True)
        self.assertEqual(symbols, rows * cols)

        generator_row = kodo.block.generator.Parity2D()
        generator_row.configure(rows, cols)
        generator_row.set_column_redundancy_enabled(False)
        self.assertEqual(generator_row.column_redundancy_enabled(), False)
        symbols = generator_row.symbols
        self.assertEqual(symbols, rows * cols)

        generator_column = kodo.block.generator.Parity2D()
        generator_column.configure(rows, cols)
        generator_column.set_row_redundancy_enabled(False)
        self.assertEqual(generator_column.row_redundancy_enabled(), False)
        symbols = generator_column.symbols
        self.assertEqual(symbols, rows * cols)

        generator_none = kodo.block.generator.Parity2D()
        generator_none.configure(rows, cols)
        generator_none.set_row_redundancy_enabled(False)
        generator_none.set_column_redundancy_enabled(False)
        self.assertEqual(generator_none.row_redundancy_enabled(), False)
        self.assertEqual(generator_none.column_redundancy_enabled(), False)

        while generator_both.can_advance():
            if generator_both.can_generate():
                coefficients, index = generator_both.generate()
            generator_both.advance()

        while generator_row.can_advance():
            if generator_row.can_generate():
                coefficients, index = generator_row.generate()
            generator_row.advance()

        while generator_column.can_advance():
            if generator_column.can_generate():
                coefficients, index = generator_column.generate()
            generator_column.advance()


if __name__ == "__main__":
    unittest.main()
