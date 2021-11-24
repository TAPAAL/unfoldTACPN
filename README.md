# unfoldTACPN

This is the extract of the unfolding part of VerifyPN specialized for timed systems.
This is intended as a temporary hot-fix for enabling support for Colored Timed-Arc Petri Nets
in the VerfifyDTAPN engine.

Over time this should be refactored into a general unfolding library, merged with
the functionality of the (faster and better) unfolder of VerifyPN.


The four distributions of unfoldTACPN can be compiled as follows
### Linux64 and OSX64
```
bzr branch lp:verifypn
mkdir build && cd  build
cmake .. -DVERIFYPN_Static=ON -DVERIFYPN_MC_Simplification=OFF

#For mac, one need to enforce that we use the GCC compiler using:
export CC=gcc-9
export CXX=g++-9
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
cmake .. -DVERIFYPN_Static=ON -DVERIFYPN_MC_Simplification=OFF -DCMAKE_TOOLCHAIN_FILE=../toolchain-x86_64-w64-mingw32.cmake
make
```

### Linux64 - Model Checking Competition
```
mkdir build
cd  build
cmake .. -DVERIFYPN_Static=OFF -DVERIFYPN_MC_Simplification=ON
make
```

### Mac 64 compilation
```
mkdir build
cd build
cmake -DVERIFYPN_MC_Simplification=OFF  -DBISON_EXECUTABLE=/usr/local/opt/bison/bin/bison -DFLEX_EXECUTABLE=/usr/local/opt/flex/bin/flex -DCMAKE_C_COMPILER=/usr/local/bin/gcc-9 -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-9 ..
make
```

