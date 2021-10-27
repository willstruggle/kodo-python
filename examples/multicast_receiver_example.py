#!/usr/bin/env python
# encoding: utf-8

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

import argparse
import kodo
import socket
import struct
import sys
import time
import os
from os import path

import atexit

MCAST_GRP = "224.1.1.1"
MCAST_PORT = 5007


def multicast_load_data_out():
    return bytearray(open("data_out", "rb").read())


def main():
    """
    Multicast example, receiver part.
    An example where data is received, decoded, and finally written to a file.
    """
    parser = argparse.ArgumentParser(description=main.__doc__)
    parser.add_argument(
        "--output-file",
        type=str,
        help="Path to the file which should be received.",
        default="output_file",
    )

    parser.add_argument(
        "--ip", type=str, help="The IP address to use.", default=MCAST_GRP
    )

    parser.add_argument("--port", type=int, help="The port to use.", default=MCAST_PORT)

    parser.add_argument(
        "--dry-run", action="store_true", help="Run without network use."
    )

    args = parser.parse_args()

    if args.dry_run:
        sys.exit(0)

    sock = socket.socket(
        family=socket.AF_INET, type=socket.SOCK_DGRAM, proto=socket.IPPROTO_UDP
    )

    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    sock.bind(("", args.port))
    mreq = struct.pack("4sl", socket.inet_aton(args.ip), socket.INADDR_ANY)

    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    decoder = None
    generator = None

    print("Processing...")
    while True:
        time.sleep(0.2)
        packet = sock.recv(10240)

        header_data = packet
        symbol = bytearray(packet[27:])

        seed, offset, width_byte, block_bytes, symbol_bytes, width = struct.unpack_from(
            "<QQBIIH", header_data
        )
        print(seed, offset, width_byte, block_bytes, symbol_bytes)
        width = kodo.perpetual.Width.from_value(width_byte)

        if (
            decoder is None
            or width != decoder.width
            or block_bytes != decoder.block_bytes
            or symbol_bytes != decoder.symbol_bytes
        ):
            decoder = kodo.perpetual.Decoder(width)
            decoder.configure(block_bytes, symbol_bytes)
            data_out = bytearray(decoder.block_bytes)
            decoder.set_symbols_storage(data_out)

        if generator is None:
            generator = kodo.perpetual.generator.RandomUniform(decoder.width)

        coefficients = generator.generate(seed)

        old_rank = decoder.rank
        decoder.decode_symbol(symbol, coefficients, offset)

        if decoder.rank != old_rank:
            print(f"Decoder rank: {decoder.rank}/{decoder.symbols}")

        if decoder.is_complete():
            break

    f = open(args.output_file, "wb")
    f.write(data_out)
    f.close()

    print("Processing finished.")


if __name__ == "__main__":
    main()
