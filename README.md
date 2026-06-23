# skWidgets

A cross-platform C++ UI framework built on [Skia](https://skia.org/) and Win32. Every pixel is rendered by Skia — there are no native OS widgets, no GLFW/SDL/SFML, and no Direct2D or GDI drawing. Win32 is used only for window creation, the message loop, and blitting the Skia framebuffer to the screen.

## Design Philosophy

- **Custom rendering** — inspired by Flutter's approach: the framework owns every pixel via `SkCanvas`, giving full control over look and feel.
- **Retained-mode widget tree** — inspired by wxWidgets: a persistent object tree where each widget holds its own state and paints itself.
- **No native controls** — widgets are drawn, not wrapped. The same widget code runs identically regardless of OS theme or DPI scaling quirks.

## Architecture

```
core/           skApp — application lifecycle
platform/       skWindow — Win32 window + message loop
rendering/      skRenderContext — SkSurface / SkCanvas ownership and presentation
ui/             widget tree, layout, events
```

### Rendering pipeline

1. `GetMessage` / Win32 message loop
2. Win32 input messages → internal `skEvent`
3. Dispatch to widget tree (`skWidget::OnEvent`)
4. Layout pass if dirty
5. Render pass: `skWidget::Paint(SkCanvas*)` on each widget
6. Present: copy `SkSurface` pixel buffer to window via `StretchDIBits` / `BitBlt`

### Key types

| Class | Role |
|---|---|
| `skApp` | Init, run, shutdown |
| `skWindow` | `CreateWindowEx`, `WndProc`, overlay + toast management |
| `skRenderContext` | Owns `SkSurface`; recreates on resize; exposes `SkCanvas*` |
| `skWidget` | Base for all controls — position, size, focus, paint, events |
| `skEvent` | Internal event type (mouse, keyboard, focus, resize, …) |
| `skSizer` / `skGridSizer` / `skFlexSizer` | Layout managers |
| `skTheme` | Colors, radii, typography tokens |

## Widget Catalog

### Core
`skWidget` · `skPanel` · `skLabel` · `skButton` · `skImage` · `skSeparator`

### Input
`skTextInput` · `skTextArea` · `skPasswordBox` · `skNumberInput` · `skCheckBox` · `skRadioButton` · `skToggle` · `skSlider` · `skDropdown` · `skDatePicker` · `skColorDialog` · `skFontDialog`

### Layout
`skSizer` · `skGridSizer` · `skFlexSizer` · `skStackPanel` · `skScrollPanel` · `skSplitView` · `skDockPanel` · `skExpander`

### Navigation
`skTabBar` · `skMenuBar` · `skMenu` · `skToolBar` · `skStatusBar` · `skSideBar` · `skNavigationRail` · `skBreadcrumb`

### Data
`skListBox` · `skTreeView` · `skTableView` · `skDataGrid` · `skPropertyGrid` · `skChartView`

### Overlays & Dialogs
`skModal` · `skMessageBox` · `skFileDialog` · `skToast` · `skTooltip` · `skPopover` · `skDrawer`

### Modern / Material
`skCard` · `skChip` · `skBadge` · `skAvatar` · `skProgressBar` · `skSpinner` · `skLink`

### Developer
`skCodeEditor` · `skConsoleView` · `skInspector` · `skCanvasView` · `skMarkdownView` · `skWebView` · `skPdfView` · `skVideoView`

## Prerequisites

| Dependency | Version | Location |
|---|---|---|
| Skia (static, CPU backend) | — | `C:\skia` (headers), `C:\skia\out\Release-windows-x64\` (libs) |
| FFmpeg development package | avcodec-62 / avutil-60 | `C:\ffmpeg` |
| WebView2 SDK | 1.0.4022.49 | auto-downloaded from NuGet, or supply `-DWEBVIEW2_SDK_DIR=…` |
| CMake | 3.29+ | — |
| MSVC | — | Windows SDK 10.0.26100.0 |

> FFmpeg runtime DLLs (`avcodec-62.dll`, `avformat-62.dll`, `avutil-60.dll`, `swresample-6.dll`, `swscale-9.dll`) are copied next to the executable automatically by the build.

## Build

```powershell
cmake -B build -S .
cmake --build build --config Release
```

Output: `build\Release\main.exe`

To point at a non-default FFmpeg location:

```powershell
cmake -B build -S . -DFFMPEG_DIR="D:\ffmpeg"
```

To supply a pre-downloaded WebView2 NuGet package (avoids the network download):

```powershell
cmake -B build -S . -DWEBVIEW2_SDK_DIR="C:\webview2-sdk"
```

## Skia notes

- CPU backend only (`SkSurfaces::Raster` with `SkImageInfo::MakeN32Premul`).
- No GPU / no `SkSurface::MakeRenderTarget`.
- Surface is recreated on every `WM_SIZE`.
- Presentation uses Win32 DIB section (`BitBlt`), not GDI drawing calls.
- Skia was built with `/MT` (static CRT); the CMake project matches this with `MultiThreaded` runtime to avoid CRT conflicts.

## C++ standard

C++17 (`/std:c++17`), MSVC, Windows x64.
