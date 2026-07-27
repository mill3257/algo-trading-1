#!/usr/bin/env python3
"""
normalize_csv.py
Converts Alpaca or Polygon historical tick exports to the format
expected by dsp_trader::core::CSVLoader:
    timestamp_ns, price, bid, ask, size, symbol

Usage:
    python scripts/normalize_csv.py --input raw_spy.csv --output data/sample/spy_ticks.csv --symbol SPY --source alpaca
"""

import argparse
import csv
import sys
from datetime import datetime, timezone


def alpaca_row_to_tick(row: dict, symbol: str) -> dict:
    """
    TODO: map Alpaca trade export columns to the normalized format.
    Alpaca trade columns (check their docs for current format):
        t (ISO timestamp), p (price), s (size)
    Bid/ask not available in trade feed — use price for both as placeholder.
    """
    raise NotImplementedError


def polygon_row_to_tick(row: dict, symbol: str) -> dict:
    """
    TODO: map Polygon tick export columns to the normalized format.
    Polygon columns vary by endpoint — check their REST docs.
    """
    raise NotImplementedError


def generate_sample_csv(output_path: str, symbol: str = "SPY", n_ticks: int = 10000) -> None:
    """
    Generates a synthetic tick CSV for testing before real data is available.
    Price follows a GBM random walk.
    """
    import random
    import math

    print(f"Generating {n_ticks} synthetic ticks → {output_path}")
    price = 450.0
    ts_ns = 1_700_000_000_000_000_000  # arbitrary start timestamp

    with open(output_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["timestamp_ns", "price", "bid", "ask", "size", "symbol"])
        for _ in range(n_ticks):
            ret = random.gauss(0, 0.0002)
            price *= math.exp(ret)
            spread = price * 0.0001
            size   = random.randint(1, 500)
            writer.writerow([
                ts_ns,
                f"{price:.4f}",
                f"{price - spread/2:.4f}",
                f"{price + spread/2:.4f}",
                size,
                symbol,
            ])
            ts_ns += random.randint(100_000, 2_000_000)  # 0.1ms – 2ms between ticks

    print("Done.")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input",   help="Input CSV from broker")
    parser.add_argument("--output",  required=True, help="Output normalized CSV")
    parser.add_argument("--symbol",  default="SPY")
    parser.add_argument("--source",  choices=["alpaca", "polygon", "sample"], default="sample")
    parser.add_argument("--n-ticks", type=int, default=10000, help="Ticks for --source sample")
    args = parser.parse_args()

    if args.source == "sample":
        generate_sample_csv(args.output, args.symbol, args.n_ticks)
        return

    if not args.input:
        print("ERROR: --input required for alpaca/polygon sources", file=sys.stderr)
        sys.exit(1)

    converter = alpaca_row_to_tick if args.source == "alpaca" else polygon_row_to_tick

    with open(args.input) as fin, open(args.output, "w", newline="") as fout:
        reader = csv.DictReader(fin)
        writer = csv.writer(fout)
        writer.writerow(["timestamp_ns", "price", "bid", "ask", "size", "symbol"])
        for row in reader:
            tick = converter(row, args.symbol)
            writer.writerow([tick["timestamp_ns"], tick["price"],
                             tick["bid"], tick["ask"], tick["size"], tick["symbol"]])

    print(f"Done → {args.output}")


if __name__ == "__main__":
    main()
