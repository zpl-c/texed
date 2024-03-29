<div align="center">
    <a href="https://github.com/zpl-c/zpl"><img src="https://user-images.githubusercontent.com/2182108/111983468-d5593e80-8b12-11eb-9c59-8c78ecc0504e.png" alt="texed" /></a>
</div>

<br />

<div align="center">
    <a href="https://discord.gg/2fZVEym"><img src="https://img.shields.io/discord/354670964400848898?color=7289DA&style=for-the-badge" alt="discord" /></a>
</div>

<br />
<div align="center">
  Stack-based texture generation tool written in C99!
</div>

<div align="center">
  <sub>
    Brought to you by <a href="https://github.com/zpl-zak">@zpl-zak</a>
    and <strong>contributors</strong>
  </sub>
</div>

# Introduction
zpl.texed is a cross-platform stack-based image generation tool suitable for prototyping textures with pixel art aesthetics. It offers a suite of tools to generate/blend/modify stacked layers and also provides an ability to export bespoke textures into PNG files or packed C header files (a header containing an array of bytes and various metadata).

**zpl.texed** runs on top of [raylib](https://raylib.com/) technologies and makes use of the [zpl](https://zpl.pw/) ecosystem alongside the **cwpack** library to provide a robust and intuitive user experience.

![texed preview](https://user-images.githubusercontent.com/9026786/118902426-eb6a5c00-b915-11eb-8569-593cbc8f4980.png)

## Features
* it's fast and cross-platform
* stack-based layer blending
* various color operations
* image generators, for instance: white noise, Perlin, Voronoi
* it offers the ability to export the resulting image to PNG or even a C header file

# How to obtain texed
## Download at itch.io
You can download this tool at [itch.io](https://zaklaus.itch.io/texed)!

**NOTE:** While GitHub sources contain the most recent development builds, the **itch.io** page is considered to be the stable channel with less frequent updates.

## Build the project
We use CMake to generate project files and manage builds.
You can do the following on the command line to create and build this project:
```sh
git clone https://github.com/zpl-c/texed.git
cd texed
cmake -S . -B build
cmake --build build
```
