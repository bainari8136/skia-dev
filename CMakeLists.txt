cmake_minimum_required(VERSION 3.29.1)
project(skia-dev)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Match the static CRT Skia was built with (/MT instead of /MD)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(SKIA_DIR "C:/skia")
set(SKIA_LIB "C:/skia/out/Release-windows-x64/skia.lib")

include_directories(${SKIA_DIR})
include_directories(${CMAKE_SOURCE_DIR})

# Prevent Windows.h from defining min/max macros that clash with Skia headers
add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)

set(SOURCES
    main.cpp
    core/skApp.cpp
    platform/skWindow.cpp
    rendering/skRenderContext.cpp
    ui/skTypeface.cpp
    ui/skButton.cpp
    ui/skLabel.cpp
    ui/skCheckBox.cpp
    ui/skSlider.cpp
    ui/skTheme.cpp
    ui/skPanel.cpp
    ui/skProgressBar.cpp
    ui/skTextInput.cpp
    ui/skRadioButton.cpp
    ui/skDropdown.cpp
    ui/skSizer.cpp
    ui/skListBox.cpp
    ui/skSeparator.cpp
    ui/skTabBar.cpp
    ui/skSpinner.cpp
    ui/skBadge.cpp
    ui/skToast.cpp
    ui/skToggle.cpp
    ui/skNumberInput.cpp
    ui/skScrollPanel.cpp
    ui/skModal.cpp
    ui/skLink.cpp
    ui/skTextArea.cpp
    ui/skStatusBar.cpp
    ui/skExpander.cpp
    ui/skPasswordBox.cpp
    ui/skImage.cpp
    ui/skCard.cpp
    ui/skTreeView.cpp
)

add_executable(main WIN32 ${SOURCES})

target_link_libraries(main
    ${SKIA_LIB}
    gdi32
    user32
    opengl32
    dwrite
)
