# dcearth

An interactive virtual globe application for the Sega Dreamcast, built using KallistiOS, to browse the Earth and view satellite imagery.

### Building

Setup the KallistiOS toolchain and ports, then run the following commands:

```
mkdir build
cd build
kos-cmake ..
cd ..
cmake --build ./build
```

To run using dcload-ip:

```
cmake --build build --target run
```

To build the disc image:

```
cmake --build build --target disc
```