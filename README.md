# unfoldTACPN

unfoldTACPN is the timed-system-specific extraction of VerifyPN's unfolding
code. It enables support for Colored Timed-Arc Petri Nets in VerifyDTAPN.

The library is intended to be replaced by a general unfolding library combining
this functionality with VerifyPN's faster unfolder.

## License

unfoldTACPN is available under the terms of the GNU GPL version 3 or (at your
option) any later version. If this license does not suit your needs, contact us
to propose an alternative license.

## Linux

Install dependencies:

```bash
sudo apt update
sudo apt install cmake ninja-build flex bison libboost-test-dev gcc-16 g++-16
```

Build a release:

```bash
cmake --workflow release
```

Run the tests:

```bash
cmake --workflow test
```

## Mac OS

Install Xcode through the App Store, then install dependencies:

```bash
brew install cmake ninja flex bison gcc@16
```

Build a release:

```bash
cmake --workflow release
```

## Windows (Cross Compile)

Install dependencies on Linux:

```bash
sudo apt update
sudo apt install cmake ninja-build flex bison mingw-w64
```

Build a Windows release:

```bash
cmake --workflow win64-release
```

## CMake Workflows

| Workflow | Purpose | Build directory |
| --- | --- | --- |
| `release` | Release build | `build-release` |
| `debug` | Debug build | `build-debug` |
| `test` | Release build and tests | `build-test` |
| `win64-release` | Windows cross-compiled release | `build-win64-release` |

The default GCC version can be overridden by setting the `GCC_VERSION` environment variable:
```bash
GCC_VERSION=12 cmake --workflow release
```
