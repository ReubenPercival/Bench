# bench

A tiny, dependency-free benchmark for **low-RAM / low-CPU devices** (old
laptops, single-board computers, cheap VPS, containers with tight limits).

It runs for about **5 minutes**, gently stresses the CPU and memory, and
prints a single **score from 10 to 100** (higher is better). No heavy
allocations, no external libraries — just one C file and pthreads.

## Build

```sh
make
```

Requires a C compiler (`cc`/`gcc`/`clang`) and pthreads. On most Linux/macOS
systems this is already available.

## Run

```sh
./bench
```

Example output:

```
bench - lightweight benchmark for low-spec devices
Run time: 300 s | Detected 4 CPUs, using 2 worker(s).

Phase 1/2: CPU test...
  done in 150.0s, 123456789 integer ops
Phase 2/2: Memory test...
  done in 150.0s, 9876543210 bytes touched

========================================
CPU rate   : 154321 ops/s/worker
Memory rate: 12.3 MB/s/worker
SCORE      : 57 / 100  (range 10-100)
========================================
```

## Options

| Option      | Description                                  | Default |
| ----------- | -------------------------------------------- | ------- |
| `-d SECONDS`| Total run time in seconds                    | `300`   |
| `-h`        | Show help                                    |         |

For a quick check on a slow device, lower the duration:

```sh
./bench -d 60
```

## How the score works

The run is split into two equal phases:

1. **CPU** — each worker runs a cheap integer linear-congruential
   generator for the duration, counting iterations.
2. **Memory** — each worker allocates a small (2 MB) buffer, touches it
   page-by-page, frees it, and repeats. Resident memory stays small.

The per-worker CPU rate and memory bandwidth are compared against modest
reference points (`50000` ops/s and `50` MB/s). CPU contributes up to 70
points and memory up to 30; the sum is clamped to **10–100**. Very weak
hardware floors at 10, strong hardware caps at 100.

## Install

```sh
sudo make install      # installs to /usr/local/bin
sudo make uninstall
```

## License

Released under the [BSD 2-Clause License](LICENSE).
