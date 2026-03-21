# dcearth

An interactive virtual globe application for the Sega Dreamcast, built using KallistiOS, to browse the Earth and view satellite imagery.

### Building

Setup the KallistiOS toolchain and ports, then run the following commands:

```
mkdir build
kos-cmake -B build -G Ninja
ninja -C build
```

To run using dcload-ip, configure with your Dreamcast's IP:

```
kos-cmake -B build -G Ninja -DDC_IP=<your-dreamcast-ip>
ninja -C build run
```

To build the disc image:

```
ninja -C build disc
```

### Development

To format code using clang-format:

```
ninja -C build format
```