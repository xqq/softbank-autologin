softbank-autologin
======
A Softbank Wi-Fi hotspot (0001softbank / mobilepoint1) login tool

## Account
Fill your softbank username & password into `account.cpp`, or the program will ask you when execute.

## Compile for Windows
#### MSVC
Install reference packages by [vcpkg](https://github.com/Microsoft/vcpkg)

```bash
vcpkg install curl[non-http]:x86-windows
```

Use cmake together with vcpkg

```bash
cmake -G "Visual Studio 15" -DCMAKE_TOOLCHAIN_FILE=<PATH_TO_VCPKG>\vcpkg\scripts\buildsystems\vcpkg.cmake
```

Or use CMake support in Visual Studio 2017, edit `CMakeSettings.json`:

```json
{
    "name": "x86-Debug",
    "generator": "Ninja",
    "configurationType": "Debug",
    "variables": [
        {
            "name": "CMAKE_TOOLCHAIN_FILE",
            "value": "<PATH_TO_VCPKG>\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake"
        }
    ]
}
```

Compile (if not use CMake support in Visual Studio 2017)

```bash
msbuild softbank_autologin.vcxproj /p:Configuration=Release
```

#### MinGW
Do it yourself

## Compile for macOS
```bash
cmake .
make -j4
```