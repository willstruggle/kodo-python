#! /usr/bin/env python
# encoding: utf-8

# Copyright Steinwurf ApS 2011-2013.
# Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
# See accompanying file LICENSE.rst or
# http://www.steinwurf.com/licensing

import kodo
import re

"""
A Simplified API for kodo python.

Usage:

    import pykodo as kodo

    algorithm = kodo.full_vector
    field = kodo.binary8

    symbols = 20
    symbol_size = 150

    encoder_factory = kodo.encoder_factory(
        algorithm, field, symbols, symbol_size)
    encoder = encoder_factory.build()

    decoder_factory = kodo.decoder_factory(
        algorithm, field, symbols, symbol_size)
    decoder = decoder_factory.build()

"""


def split_upper_case(s):
    return [a for a in re.split(r'([A-Z][a-z]*\d*)', s) if a]


def __get_stacks():
    result = {}
    for stack in dir(kodo):
        if stack.startswith('__'):
            continue

        stack_pieces = split_upper_case(stack)
        trace = 'no_trace'
        if stack_pieces[-1] == 'Trace':
            trace = 'trace'
            stack_pieces.pop()
        field = stack_pieces.pop().lower()

        if stack_pieces[-1] != "Factory":
            continue
        stack_pieces.pop()
        coder_type = stack_pieces.pop().lower()

        algorithm = "_".join(stack_pieces).lower()

        if algorithm not in result:
            result[algorithm] = {}
        if field not in result[algorithm]:
            result[algorithm][field] = {}
        if coder_type not in result[algorithm][field]:
            result[algorithm][field][coder_type] = {}

        result[algorithm][field][coder_type][trace] = getattr(kodo, stack)
    return result


__kodo_stacks = __get_stacks()

algorithms = []
for algorithm in __kodo_stacks:
    globals()[algorithm] = algorithm
    algorithms.append(algorithm)

fields = []
for field in list(__kodo_stacks.items())[0][1]:
    globals()[field] = field
    fields.append(field)

globals()['trace'] = True
globals()['no_trace'] = False


def decoder_factory(algorithm, field, max_symbols, max_symbol_size,
                    trace=False):
    trace = 'trace' if trace else 'no_trace'
    return __kodo_stacks[algorithm][field]['decoder'][trace](
        max_symbols, max_symbol_size)


def encoder_factory(algorithm, field, max_symbols, max_symbol_size,
                    trace=False):
    trace = 'trace' if trace else 'no_trace'
    return __kodo_stacks[algorithm][field]['encoder'][trace](
        max_symbols, max_symbol_size)
