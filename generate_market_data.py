#!/usr/bin/env python3
"""Generate a synthetic HFT CSV dataset quickly."""

from __future__ import annotations

import random
from pathlib import Path


OUTPUT_PATH = Path("market_data.csv")
ROW_COUNT = 1_000_000
START_MID_PRICE = 10_000
MIN_MID_PRICE = 100
MIN_OFFSET = 1
MAX_OFFSET = 5
CHUNK_SIZE = 100_000


def generate_market_data() -> None:
    rng = random.Random()
    mid_price = START_MID_PRICE

    randrange = rng.randrange
    choices = ("B", "S")

    with OUTPUT_PATH.open("w", buffering=1 << 20, newline="") as output:
        for chunk_start in range(1, ROW_COUNT + 1, CHUNK_SIZE):
            chunk_end = min(chunk_start + CHUNK_SIZE, ROW_COUNT + 1)
            rows: list[str] = []
            append = rows.append

            for order_id in range(chunk_start, chunk_end):
                mid_price = max(
                    MIN_MID_PRICE,
                    mid_price
                    + (randrange(1, 11) if randrange(2) else -randrange(1, 11)),
                )
                side = choices[randrange(2)]
                offset = randrange(MIN_OFFSET, MAX_OFFSET + 1)
                price = mid_price - offset if side == "B" else mid_price + offset

                append(
                    f"{order_id},{side},{price},{randrange(1, 101)},L\n"
                )

            output.writelines(rows)


if __name__ == "__main__":
    generate_market_data()
