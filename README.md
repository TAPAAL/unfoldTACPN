# unfoldTACPN

This is the extract of the unfolding part of VerifyPN specialized for timed systems.
This is intended as a temporary hot-fix for enabling support for Colored Timed-Arc Petri Nets
in the VerfifyDTAPN engine.

Over time this should be refactored into a general unfolding library, merged with
the functionality of the (faster and better) unfolder of VerifyPN.


The four distributions of unfoldTACPN can be compiled as follows
### Linux64 and OSX64
```
mkdir build && cd  build
cmake ..

#For mac, one need to enforce that we use the GCC compiler using:
export CC=gcc-13
export CXX=g++-13
#and point to the correct version of flex and bison by adding
#-DBISON_EXECUTABLE=/usr/local/opt/bison/bin/bison -DFLEX_EXECUTABLE=/usr/local/opt/flex/bin/flex
#to cmake call

```

### Windows 64 cross-compilation with minGW
Install cross-compiler and libs

```
sudo apt install mingw-w64-x86-64-dev mingw-w64-tools g++-mingw-w64-x86-64
sudo apt install wine wine-binfmt #Needed to run tests compile
```

To build

```
mkdir build-win && cd  build-win
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain-x86_64-w64-mingw32.cmake
make
```

### Mac 64 compilation
```
mkdir build
cd build
cmake -DBISON_EXECUTABLE=/usr/local/opt/bison/bin/bison -DFLEX_EXECUTABLE=/usr/local/opt/flex/bin/flex -DCMAKE_C_COMPILER=/usr/local/bin/gcc-9 -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-9 ..
make
```

## License
VerifyPN is available under the terms of the GNU GPL version 3 or (at your option) any later version.
If this license doesn't suit you're welcome to contact us, and purpose an alternative license.
