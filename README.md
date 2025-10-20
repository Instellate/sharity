# Sharity
a P2P E2EE cross compilable file sharing platform for 1on1 file sharing.

## Building

#### Dependencies
- Qt6
- [LibDatachannel](https://github.com/paullouisageneau/libdatachannel)
- Rust

Currently to build the client you will first need to build the `vodozemac` library. 
The client depends on this library to ensure E2EE. This library only need to be built once to generate appropriate headers.
```sh
$ cd vodozemac
$ cargo build
```

### Linux
```sh
$ cd client
$ cmake -B build -GNinja -DCMAKE_BUILD_TYPE=Release
$ cmake --build
```

### Windows
Windows has some issues. It requires manual building for the `libdatachannel` library. Building `libdatachannel` on windows can be found in the GitHub workflow.  
This requires at least Visual Studio 2022 to be installed.
MSVC is the only successfull way the client has been built on Windows so far.  
Change the directory specified by `Qt6_DIR` to your locally installed directory.

```sh
$ cd client
$ cmake -B build -G "Visual Studio 17 2022" -DQt6_DIR="C:\Qt\6.9.1\msvc2022_64" -DCMAKE_BUILD_TYPE=Release
$ cmake --build build --config Release
```

### Mac
The client has yet been built and tested on Mac so far.