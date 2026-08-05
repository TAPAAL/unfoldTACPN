# unfoldTACPN

This is the extract of the unfolding part of VerifyPN specialized for timed systems.
This is intended as a temporary hot-fix for enabling support for Colored Timed-Arc Petri Nets
in the VerfifyDTAPN engine.

Over time this should be refactored into a general unfolding library, merged with
the functionality of the (faster and better) unfolder of VerifyPN.


## CMake Workflows

| Workflow | Purpose | Build directory |
| --- | --- | --- |
| `release` | Release build | `build-release` |
| `debug` | Debug build | `build-debug` |
| `test` | Release build and tests | `build-test` |
| `win64-release` | Windows cross-compiled release | `build-win64-release` |

```sh
cmake --workflow --preset <workflow>
```

### Dependencies (Linux / macOS)

```sh
sudo apt install flex bison ninja-build gcc-16 g++-16
```

### Dependencies (Windows cross-compilation with MinGW)

```sh
sudo apt install flex bison ninja-build mingw-w64
```

## License
VerifyPN is available under the terms of the GNU GPL version 3 or (at your option) any later version.
If this license doesn't suit you're welcome to contact us, and purpose an alternative license.
