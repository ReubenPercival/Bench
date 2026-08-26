# Contributing

Thanks for your interest in improving `bench`!

## Getting started

```sh
make          # build the ./bench binary
./bench -d 30 # quick smoke test
make clean    # remove build artifacts
```

## Guidelines

- Keep it **dependency-free** and **lightweight**. The whole point of this
  tool is to run on weak hardware with little RAM/CPU. Avoid heavy libraries.
- Single C file (`bench.c`) is intentional — please keep the implementation
  in one place unless there is a strong reason not to.
- Follow the existing code style: 4-space indentation, no tabs, clear names.
- Build must stay warning-free with `make` (`-Wall -Wextra`).
- Update `README.md` if you change behavior or options.

## Reporting issues / sending patches

- Open an issue describing the device, OS, and what went wrong.
- Pull requests are welcome; please explain the motivation and keep changes
  focused.

By contributing you agree your code is released under the BSD 2-Clause
License.
