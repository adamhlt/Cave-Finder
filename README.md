![Banner](https://user-images.githubusercontent.com/48086737/170099876-7e440a35-9fe5-474a-979a-e9955d5f1e4b.png)

# Cave Finder

[![C++](https://img.shields.io/badge/language-C%2B%2B-%23f34b7d.svg?style=for-the-badge&logo=appveyor)](https://en.wikipedia.org/wiki/C%2B%2B) [![Windows](https://img.shields.io/badge/platform-Windows-0078d7.svg?style=for-the-badge&logo=appveyor)](https://en.wikipedia.org/wiki/Microsoft_Windows) [![x86](https://img.shields.io/badge/arch-x86-red.svg?style=for-the-badge&logo=appveyor)](https://en.wikipedia.org/wiki/X86) [![x64](https://img.shields.io/badge/arch-x64-green.svg?style=for-the-badge&logo=appveyor)](https://en.wikipedia.org/wiki/X64)

## :open_book: Project Overview :

This tool help to find code caves in PE images (exe, dll, ...).

Retrieved informations :

- Section whe the cave reside.
- Size of the cave.
- Start / End file offset of the cave.
- Start / End virtual address of the cave
- Permission on the cave (read / write / execute).

The program retrieve free space in the file (array of 0x0), in every sections of a PE image. With enough space, you can write a shellcode...

This project can be compiled for x86 and x64 architecture.

## :rocket: Getting Started :

### Visual Studio :

1. Open the solution file (.sln).
2. Build the project in Debug / Release (x86 / x64)

### Other IDE using CMAKE :

This **CMakeLists.txt** should compile the project.

```cmake
cmake_minimum_required(VERSION 3.0)
project(miner)

set(CMAKE_CXX_STANDARD 17)

add_executable(miner main.cpp)
```

Tested on CLion with MSVC compiler, you can get Visual Studio Build Tools [**here**](https://visualstudio.microsoft.com/fr/downloads/?q=build+tools).

## 🧪 Usage :

### How to use the program :

Use it in the command line :

```shell
miner.exe <source image> <code cave size>
```

### Demonstration :
https://user-images.githubusercontent.com/48086737/158227625-db6beb29-da68-4d90-9438-e737bff332a7.mp4
