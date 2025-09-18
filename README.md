# Mine Escape

A C++17 console program that simulates escaping a mine by clearing the least-difficult neighboring tiles, handling TNT chain reactions, and reporting statistics. Supports reading a fully specified map (M mode) or procedurally generating a map (R mode) using a Mersenne Twister PRNG.

## Build

Requires:
- g++ with C++17 support
- make

Common targets:
- `make release` → optimized binary `mineEscape`
- `make debug` → sanitized debug binary `mineEscape_debug`
- `make clean` → remove build artifacts

## Run

Program reads input from stdin and writes to stdout. Two input modes are supported.

- M mode (Map provided):
  First line `M`, followed by lines:
  - `Size: <N>`
  - `Start: <row> <col>`
  - N lines of N integers where `-1` denotes TNT and non-negative integers are rubble difficulty

- R mode (Randomly generated):
  First line `R`, followed by lines:
  - `Size: <N>`
  - `Start: <row> <col>`
  - `Seed: <uint>`
  - `Max_Rubble: <uint>`
  - `TNT: <uint>` (chance denominator; 0 disables TNT)

### CLI options
- `-h, --help`    print help and exit
- `-s N, --stats N`  print first/last/easiest/hardest N cleared tiles
- `-m, --median`  print rolling median after each clear
- `-v, --verbose` print each clear/TNT event as it occurs

### Examples
Run with provided spec inputs:
```bash
make release
./mineEscape < spec-M-in.txt
./mineEscape -m -v -s 3 < spec-R-in.txt
```

Sample input (M mode):
```
M
Size: 5
Start: 1 2
9 0 9 3 3
6 9 6 8 3
9 4 1 9 0
2 0 -1 -1 9
8 3 9 7 5
```

Output includes a final summary, e.g.:
```
Cleared 6 tiles containing 41 rubble and escaped.
```
With `-m -v -s 3`, verbose events, medians, and stats sections are printed (see `spec-msv-out.txt`).

## Testing

- Golden tests: compare your program output with the provided `*-out.txt` examples
```bash
./mineEscape -m -v -s 8 < spec-M-in.txt | diff -u - spec-msv-out.txt
```
- Try different seeds and parameters in R mode to exercise TNT chains and medians.

## Implementation Notes

- Priority-driven expansion chooses the smallest rubble, then smaller column, then smaller row.
- TNT (`-1`) triggers a prioritized flood that clears neighbors and can chain.
- Running medians are computed using two heaps.

## Files
- `main.cpp` — CLI, parsing, solver
- `P2random.h/.cpp` — PRNG and random map generation
- `Makefile` — build, debug, and packaging targets
- `spec-*-in.txt`, `*-out.txt` — example inputs/outputs
