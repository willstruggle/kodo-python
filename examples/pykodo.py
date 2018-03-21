#! /usr/bin/env python
# encoding: utf-8

# Copyright Steinwurf ApS 2015.
# Distributed under the "STEINWURF EVALUATION LICENSE 1.0".
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
        algorithm=algorithm,
        field=field,
        max_symbols=symbols,
        max_symbol_size=symbol_size)
    encoder = encoder_factory.build()

    decoder_factory = kodo.decoder_factory(
        algorithm=algorithm,
        field=field,
        max_symbols=symbols,
        max_symbol_size=symbol_size)
    decoder = decoder_factory.build()

"""


def nested_add(dictionary, keys, value):
    current_dict = dictionary
    for key in keys:
        new_dict = {}
        if key == keys[-1]:
            current_dict[key] = value
            continue
        if key not in current_dict:
            current_dict[key] = new_dict
            current_dict = new_dict
        else:
            current_dict = current_dict[key]


def nested_get(dictionary, keys):
    current_dict = dictionary

    for key in keys:
        if key in current_dict:
            value = current_dict[key]
            if type(value) == dict:
                current_dict = value
            else:
                return value
        else:
            raise KeyError("{} not found.".format(" ".join(keys)))


def split_upper_case(s):
    return [a for a in re.split(r'([A-Z][a-z]*\d*)', s) if a]


def __get_stacks():

    kodo_stacks = {}
    algorithms = []
    fields = []

    for stack in dir(kodo):
        if stack.startswith('__'):
            continue

        stack_pieces = split_upper_case(stack)

        # NoCode does not have a field
        field = None
        if "".join(stack_pieces[:2]) != "NoCode":
            field = stack_pieces.pop().lower()
            if field not in fields:
                fields.append(field)

        if stack_pieces[-1] != "Factory":
            continue
        stack_pieces.pop()
        coder_type = stack_pieces.pop().lower()

        algorithm = "_".join(stack_pieces).lower()
        if algorithm not in algorithms:
            algorithms.append(algorithm)

        location = [algorithm]

        if field is not None:
            location += [field]

        location += [coder_type]

        nested_add(
            kodo_stacks,
            location,
            getattr(kodo, stack))

    return (kodo_stacks, algorithms, fields)


__kodo_stacks, algorithms, fields = __get_stacks()

for algorithm in algorithms:
    globals()[algorithm] = algorithm

for field in fields:
    globals()[field] = field


def __create_factory(algorithm, coder_type, max_symbols,
                     max_symbol_size, field=None):
    location = [algorithm]
    if field is not None:
        location += [field]
    location += [coder_type]

    return nested_get(__kodo_stacks, location)(max_symbols, max_symbol_size)


def decoder_factory(**kwargs):
    """
    Return a decoder factory matching the given arguments.

    Note: only keyword arguments are allowed for this function.

        :param algorithm: The algorithm to use (required).
        :param field: The field to use (required for certain stacks).
        :param max_symbols: The maximum number of symbols for the factory's
                            decoders.
        :param max_symbol_size: The maximum size of the symbols for the
                                factory's decoders.
    """
    return __create_factory(coder_type="decoder", **kwargs)


def encoder_factory(**kwargs):
    """
    Return an encoder factory matching the given arguments.

    Note: only keyword arguments are allowed for this function.

        :param algorithm: The algorithm to use (required).
        :param field: The field to use (required for certain stacks).
        :param max_symbols: The maximum number of symbols for the factory's
                            encoders.
        :param max_symbol_size: The maximum size of the symbols for the
                                factory's encoders.
    """
    return __create_factory(coder_type="encoder", **kwargs)
