# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is **skWidgets** — a prototype cross-platform C++ UI framework built on top of Skia (Google's 2D graphics engine) and Win32. It renders all UI using Skia; Win32 is used only for windowing and input. There are no native OS widgets, no GLFW/SDL/SFML, and no Direct2D or GDI for drawing (GDI is only used to blit the Skia framebuffer to the window).

## Build

**Prerequisites:**
- Skia built at `C:\skia` (include headers) and `C:\skia\out\Release-windows-x64\` (static libs)
- CMake 3.29+, MSVC (cl.exe), Windows SDK 10.0.26100.0

**Configure and build:**
```powershell
cmake -B build -S .
cmake --build build --config Release
```

The output executable is `build\Release\main.exe`. There is no test suite yet.

## Architecture

The intended architecture (from `skwidgets.md`) has four layers:

| Layer | Responsibility |
|-------|---------------|
| **Core** (`/core`) | Application lifecycle (`skApp`) |
| **Platform** (`/platform`) | Win32 window + message loop (`skWindow`) |
| **Rendering** (`/rendering`) | Skia wrapper (`skRenderContext`) — owns `SkSurface` + `SkCanvas` |
| **UI** (`/ui`) | Widget tree, layout, events (`skWidget`, `skButton`, `skEvent`, `skSizer`) |

Currently all code lives in `main.cpp` (prototype stage). The goal is to evolve it into the layered structure above.

### Rendering pipeline

1. `GetMessage` / Win32 message loop
2. Translate Win32 input messages → internal `skEvent`
3. Dispatch to widget tree
4. Layout pass (if dirty)
5. Render pass: call `widget->Paint(canvas)` on each widget
6. Present: copy `SkSurface` pixel buffer to window via `StretchDIBits` / `BitBlt`

### Key classes to implement

```cpp
class skWidget   { virtual void Paint(SkCanvas*); virtual void OnEvent(const skEvent&); };
class skButton   : public skWidget { /* hover + click state, color change */ };
class skRenderContext { sk_sp<SkSurface> surface; /* resize, clear, flush */ };
class skWindow   { /* CreateWindowEx, WndProc, message loop */ };
class skApp      { /* init, run, shutdown */ };
```

### Skia usage rules

- CPU backend only (`SkSurfaces::Raster` with `SkImageInfo::MakeN32Premul`)
- No GPU / no `SkSurface::MakeRenderTarget`
- Surface must be recreated on `WM_SIZE`
- Present via Win32 DIB section (`CreateDIBSection` → `BitBlt`), not GDI drawing

## Skia paths (local machine)

- Headers: `C:\skia`
- Static libs: `C:\skia\out\Release-windows-x64\` (skia.lib + supporting libs already copied to `build\Release\`)

## C++ standard

C++17 (`/std:c++17`), MSVC, Windows x64.
