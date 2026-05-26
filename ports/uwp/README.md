# wipEout UWP Wrapper

This is the Xbox Dev Mode wrapper for wipEout.

The game builds as `wipeout.dll` with `clang-cl`. The UWP project is just the
small launcher, DLL staging, manifest bits and MSIX.

## 1. Tools

Install Visual Studio 2022 with the UWP and Windows app tools.

Install CMake, Ninja and LLVM if they are not already on your machine:

```powershell
winget install Kitware.CMake
winget install Ninja-build.Ninja
winget install LLVM.LLVM
```

Make sure these are on `PATH` in a fresh terminal:

```text
cmake
ninja
clang-cl
```

LLVM usually puts `clang-cl` here:

```text
C:\Program Files\LLVM\bin
```

## 2. Dependencies

Install the UWP bits into vcpkg:

```powershell
vcpkg install `
  sdl2:x64-uwp `
  angle:x64-uwp
```

Set `VCPKG_ROOT` before configuring either build:

```powershell
$env:VCPKG_ROOT="<vcpkg-root>"
```

The build uses vcpkg for:

```text
SDL2 headers, import libs and SDL2.dll
ANGLE headers and import libs
```

The MSIX does not package ANGLE's vcpkg runtime DLLs. It packages the patched
Xbox UWP ANGLE runtime that is already in this repo:

```text
ports/uwp/third_party/angle
```

That folder contains:

```text
libEGL.dll
libGLESv2.dll
d3dcompiler_47.dll
LICENSE
AUTHORS
```

The patched ANGLE runtime creates an Xbox sized EGL surface. The game asks EGL
for the real surface size through:

```text
ports/uwp/helpers/uwp_egl_surface_size.c
```

If you need to override the dependency roots manually, pass:

```text
-DWIPEOUT_UWP_SDL_ROOT="<vcpkg-root>/installed/x64-uwp"
-DWIPEOUT_UWP_ANGLE_ROOT="<vcpkg-root>/installed/x64-uwp"
```

## 3. Build Environment

Use a Visual Studio developer command prompt for UWP before running the build
commands. `clang-cl` still needs the MSVC and Windows SDK environment, and the
UWP app platform matters.

```powershell
$env:VCPKG_ROOT="<vcpkg-root>"

& "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" `
  -arch=x64 `
  -host_arch=x64 `
  -app_platform=UWP
```

If you skip the `-app_platform=UWP` bit, the DLL can link against the desktop
VC runtime and Xbox will not like that.

## 4. Build the Game DLL

```powershell
cmake -S . -B build/uwp-wipeout-clang -G Ninja `
  "-DCMAKE_C_COMPILER=clang-cl" `
  "-DCMAKE_CXX_COMPILER=clang-cl" `
  "-DCMAKE_SYSTEM_NAME=WindowsStore" `
  "-DCMAKE_SYSTEM_VERSION=10.0.19041.0" `
  "-DCMAKE_BUILD_TYPE=Release" `
  "-DUWP_BUILD=ON" `
  "-DPLATFORM=SDL2" `
  "-DRENDERER=GLES2"

cmake --build build/uwp-wipeout-clang --target wipeout
```

That gives you:

```text
build/uwp-wipeout-clang/wipeout.dll
build/uwp-wipeout-clang/wipeout.lib
```

Quick runtime check:

```powershell
dumpbin /dependents build/uwp-wipeout-clang/wipeout.dll
```

You want to see:

```text
VCRUNTIME140_APP.dll
```

## 5. Build the Package

```powershell
cmake -S ports/uwp -B build/uwp-package -G "Visual Studio 17 2022" `
  "-DCMAKE_SYSTEM_NAME=WindowsStore" `
  "-DCMAKE_SYSTEM_VERSION=10.0.19041.0" `
  "-DWIPEOUT_DLL_DIR=build/uwp-wipeout-clang"

cmake --build build/uwp-package --config Release --target wipeout-uwp
```

The package lands here:

```text
build/uwp-package/AppPackages
```

The useful file is the `.msix` under the `wipeout-uwp_1.0.0.0_x64_Test`
folder.

To deploy from Visual Studio, open:

```text
build/uwp-package/wipeout-uwp.sln
```

Use:

```text
Configuration: Release
Platform: x64
Startup project: wipeout-uwp
```

## 6. Game Data

Shared packages should not include the game data.

Put user supplied data in LocalState like this:

```text
LocalState/
  wipeout/
    textures/drfonts.cmp
    common/
    music/
```

For a private package with data included, pass this when configuring the package:

```text
-DWIPEOUT_DATA_ROOT="<path-to-wipeout-game-data>"
```
