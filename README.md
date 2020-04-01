[![CodeFactor](https://www.codefactor.io/repository/github/reissgrvs/gb-step/badge/master?s=71772b3c5f8647120cafa712801daccfb06f4f7a)](https://www.codefactor.io/repository/github/reissgrvs/gb-step/overview/master)

# gb-step

Gameboy Advance Emulator for final year project

# Build Instructions

Build using CMake.
To build from gb-step directory:
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make -j
```
To run from build directory:
```
./gba {PATH_TO_BIOS} {PATH_TO_ROM}
```

# Requirements  
CMake 3.10+  
C++17 compliant compiler  
SFML 2.3+ (https://www.sfml-dev.org/)  
  
DEBUG ONLY: spdlog (https://github.com/gabime/spdlog)  


