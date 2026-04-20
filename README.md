# netcalc

Small command-line subnet calculators for IPv4 and IPv6.

Current release: `v1.0.0`

This project builds two user-facing tools:

- `ipv4calc`: prints IPv4 subnet information from an address plus CIDR prefix or dotted netmask
- `ipv6calc`: prints IPv6 subnet information from an address plus CIDR prefix

Internally, both CLIs share a small `libnetcalc` library for common option parsing,
output routing, and text/JSON/XML rendering. The serializers now live in a
dedicated internal formatting module, `netcalc_format`.

## Features

Both tools support:

- `--help`
- `--version`
- `--output=stdout`
- `--output=stderr`
- `--output=<file path>`
- `--format json`
- `--format xml`

`ipv4calc` reports:

- IP address
- netmask and wildcard mask
- binary netmask
- prefix length and CIDR notation
- network and broadcast address
- usable host range
- total and usable host counts
- IPv4 class

`ipv6calc` reports:

- compressed and expanded IPv6 address
- compressed and expanded prefix mask
- prefix length and CIDR notation
- network address
- first and last address in the subnet
- total address count
- coarse address type

## Requirements

- C++11 compiler for the production binaries and internal library
- C++17 compiler only if you want to build the test targets
- autotools if building with the automake path
- CMake 3.16+ if building with the CMake path
- `googletest` with `gtest`/`gmock` available if you want to run tests

The calculators do not require external JSON or XML libraries. Both `--format json`
and `--format xml` are implemented internally.

## Build With Autotools

```bash
./autogen.sh
./configure
make
```

Build outputs:

- `src/ipv4calc`
- `src/ipv6calc`
- `src/.libs/libnetcalc.dylib` (macOS) / `src/.libs/libnetcalc.so` (Linux)
- `src/.libs/libnetcalc.a` (libtool static archive)

Install:

```bash
make install
```

Run distribution checks:

```bash
make dist-check
```

Run the release target:

```bash
make release
```

To install into a custom prefix:

```bash
./configure --prefix=/usr/local
make
make install
```

To stage an install for packaging:

```bash
make install DESTDIR=/tmp/ipv4calc-package
```

Run tests:

```bash
make check
```

If the compiler does not support C++17 or `googletest` is not installed, the
production binaries still build and `make check` simply has no test programs to run.

Autotools also installs `libnetcalc.pc` into `lib/pkgconfig/` so downstream builds
can discover the public library with `pkg-config`.

## Build With CMake

```bash
cmake -S . -B build
cmake --build build
```

Build outputs:

- `build/ipv4calc`
- `build/ipv6calc`
- `build/libnetcalc` shared library (`.dylib` on macOS, `.so` on Linux)

To build a static library instead:

```bash
cmake -S . -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build
```

Install:

```bash
cmake --install build
```

To install into a custom prefix:

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
cmake --install build
```

To stage an install for packaging:

```bash
cmake --install build --prefix /tmp/ipv4calc-package/usr/local
```

Run tests:

```bash
ctest --test-dir build --output-on-failure
```

If C++17 support or GTest is unavailable, CMake skips the test targets and still
builds the production binaries.

CMake installs the same `libnetcalc.pc` file under `lib/pkgconfig/`.

## Usage

### IPv4

CIDR form:

```bash
src/ipv4calc 192.168.1.10/24
```

Address plus netmask:

```bash
src/ipv4calc 172.16.54.24 255.255.252.0
```

Single host:

```bash
src/ipv4calc 8.8.4.4
```

JSON output:

```bash
src/ipv4calc --format json 192.168.1.10/24
```

XML output:

```bash
src/ipv4calc --format xml 192.168.1.10/24
```

Write output to a file:

```bash
src/ipv4calc --output=result.txt 192.168.1.10/24
```

Explicit stdout output:

```bash
src/ipv4calc --format json --output=stdout 192.168.1.10/24
```

Explicit stderr output:

```bash
src/ipv4calc --format json --output=stderr 192.168.1.10/24
```

### IPv6

CIDR form:

```bash
src/ipv6calc 2001:db8:abcd:12::1234/64
```

Address plus prefix:

```bash
src/ipv6calc fd12:3456:789a::42 56
```

Single host:

```bash
src/ipv6calc 2001:db8::1
```

JSON output:

```bash
src/ipv6calc --format json 2001:db8:abcd:12::1234/64
```

XML output:

```bash
src/ipv6calc --format xml 2001:db8:abcd:12::1234/64
```

Write output to a file:

```bash
src/ipv6calc --output=result.txt 2001:db8:abcd:12::1234/64
```

Explicit stdout output:

```bash
src/ipv6calc --format xml --output=stdout 2001:db8:abcd:12::1234/64
```

Explicit stderr output:

```bash
src/ipv6calc --format xml --output=stderr 2001:db8:abcd:12::1234/64
```

## Versioning

`--version` is generated at build time. The version string preference order is:

- `NETCALC_VERSION`
- `GITHUB_REF_NAME`
- short `GITHUB_SHA`
- local `git rev-parse --short HEAD`
- project version fallback

This makes local builds and GitHub release builds report useful version metadata
without maintaining a hardcoded version header.

For packaged and source-release builds, the project version fallback is `1.0.0`.

## Installed Files

By default, installation places files in the usual platform locations:

- binaries in `bin/`
- the static library in `lib/`
- public headers in `include/`

Installed artifacts currently include:

- `ipv4calc`
- `ipv6calc`
- `libnetcalc.a`
- `libnetcalc.pc`
- `netcalc.h`
- `netcalc_format.h`
- `ipv4calc.h`
- `ipv6calc.h`

## pkg-config

Because `libnetcalc` is now a public library, installation also provides
`libnetcalc.pc`.

Example use:

```bash
pkg-config --cflags --libs libnetcalc
```

Typical output will point consumers at the installed headers and static library.

## Testing

The test suite currently includes:

- shared library tests for `libnetcalc`
- formatter-level tests for JSON/XML serialization and output-safety behavior
- IPv4 unit tests
- IPv4 CLI integration tests
- IPv6 unit tests
- IPv6 CLI integration tests

CLI coverage includes:

- valid subnet output
- invalid input handling
- `--help`
- `--version`
- `--format json`
- `--format xml`
- `--output=stdout`
- `--output=stderr`
- `--output=<file path>`

## Continuous Integration

GitHub Actions CI is configured in [`.github/workflows/ci.yml`](.github/workflows/ci.yml).

It currently runs:

- `autotools` builds and tests
- `cmake` builds and tests
- `ubuntu-latest`
- `macos-latest`

## Project Layout

- [`src/netcalc.h`](src/netcalc.h): shared internal library API
- [`src/netcalc.cpp`](src/netcalc.cpp): shared option parsing and output routing
- [`src/netcalc_format.h`](src/netcalc_format.h): formatter API for text, JSON, and XML output
- [`src/netcalc_format.cpp`](src/netcalc_format.cpp): text/JSON/XML serialization helpers
- [`src/main.cpp`](src/main.cpp): `ipv4calc` CLI entry point
- [`src/ipv4calc.cpp`](src/ipv4calc.cpp): IPv4 parsing and subnet helpers
- [`src/ipv6_main.cpp`](src/ipv6_main.cpp): `ipv6calc` CLI entry point
- [`src/ipv6calc.cpp`](src/ipv6calc.cpp): IPv6 parsing and subnet helpers
- [`tests/test_netcalc.cpp`](tests/test_netcalc.cpp): shared library tests
- [`tests/test_ipv4calc.cpp`](tests/test_ipv4calc.cpp): IPv4 unit tests
- [`tests/test_ipv4calc_cli.cpp`](tests/test_ipv4calc_cli.cpp): IPv4 CLI integration tests
- [`tests/test_ipv6calc.cpp`](tests/test_ipv6calc.cpp): IPv6 unit tests
- [`tests/test_ipv6calc_cli.cpp`](tests/test_ipv6calc_cli.cpp): IPv6 CLI integration tests

## Notes

- Invalid prefixes are validated before subnet math is performed.
- The IPv6 tool intentionally does not print IPv4-only concepts such as broadcast or wildcard addresses.
- JSON and XML output are built in and do not depend on external serialization libraries.
- Both build systems now provide install targets for `ipv4calc`, `ipv6calc`, `libnetcalc.a`, and the public headers.
- The top-level autotools build now provides explicit `make dist-check` and `make release` targets for release validation under the `netcalc` package name.
