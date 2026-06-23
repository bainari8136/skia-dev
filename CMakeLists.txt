cmake_minimum_required(VERSION 3.29.1)
project(skia-dev)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Match the static CRT Skia was built with (/MT instead of /MD)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(SKIA_DIR "C:/skia")
set(SKIA_LIB "C:/skia/out/Release-windows-x64/skia.lib")
set(FFMPEG_DIR "C:/ffmpeg" CACHE PATH "FFmpeg development package root")

if(NOT EXISTS "${FFMPEG_DIR}/include/libavcodec/avcodec.h")
    message(FATAL_ERROR "FFmpeg development headers were not found under ${FFMPEG_DIR}")
endif()

include_directories(${SKIA_DIR})
include_directories(${CMAKE_SOURCE_DIR})

# WebView2 SDK. A local unpacked SDK can be supplied with
# -DWEBVIEW2_SDK_DIR=...; otherwise the pinned official NuGet package is
# downloaded once into the build tree.
set(WEBVIEW2_VERSION "1.0.4022.49")
set(WEBVIEW2_SDK_DIR "" CACHE PATH "Path to an unpacked Microsoft.Web.WebView2 NuGet package")
if(NOT WEBVIEW2_SDK_DIR)
    if(EXISTS "${CMAKE_SOURCE_DIR}/.deps/webview2/build/native/include/WebView2.h")
        set(WEBVIEW2_SDK_DIR "${CMAKE_SOURCE_DIR}/.deps/webview2")
    else()
        set(WEBVIEW2_SDK_DIR "${CMAKE_BINARY_DIR}/_deps/webview2")
        set(WEBVIEW2_PACKAGE "${CMAKE_BINARY_DIR}/_deps/Microsoft.Web.WebView2.${WEBVIEW2_VERSION}.nupkg")
        if(NOT EXISTS "${WEBVIEW2_SDK_DIR}/build/native/include/WebView2.h")
            file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/_deps")
            file(DOWNLOAD
                "https://api.nuget.org/v3-flatcontainer/microsoft.web.webview2/${WEBVIEW2_VERSION}/microsoft.web.webview2.${WEBVIEW2_VERSION}.nupkg"
                "${WEBVIEW2_PACKAGE}"
                EXPECTED_HASH "SHA256=EE9DE67E5BB9EF3A96C5689B2EFC8188E2DF160A0E79234C0404243782FDE5FB"
                TLS_VERIFY ON
                SHOW_PROGRESS)
            file(ARCHIVE_EXTRACT INPUT "${WEBVIEW2_PACKAGE}" DESTINATION "${WEBVIEW2_SDK_DIR}")
        endif()
    endif()
endif()

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
    ui/skChip.cpp
    ui/skAvatar.cpp
    ui/skToolBar.cpp
    ui/skBreadcrumb.cpp
    ui/skSideBar.cpp
    ui/skPopover.cpp
    ui/skMenu.cpp
    ui/skMenuBar.cpp
    ui/skMessageBox.cpp
    ui/skGridSizer.cpp
    ui/skTableView.cpp
    ui/skPropertyGrid.cpp
    ui/skDrawer.cpp
    ui/skNavigationRail.cpp
    ui/skSplitView.cpp
    ui/skColorDialog.cpp
    ui/skChartView.cpp
    ui/skDatePicker.cpp
    ui/skConsoleView.cpp
    ui/skDataGrid.cpp
    ui/skFlexSizer.cpp
    ui/skCanvasView.cpp
    ui/skCodeEditor.cpp
    ui/skFileDialog.cpp
    ui/skStackPanel.cpp
    ui/skFontDialog.cpp
    ui/skMarkdownView.cpp
    ui/skInspector.cpp
    ui/skDockPanel.cpp
    ui/skTooltip.cpp
    ui/skVideoView.cpp
    ui/skWebView.cpp
    ui/skPdfView.cpp
)

add_executable(main WIN32 ${SOURCES})

target_include_directories(main PRIVATE "${WEBVIEW2_SDK_DIR}/build/native/include")
target_include_directories(main PRIVATE "${FFMPEG_DIR}/include")

target_link_libraries(main
    ${SKIA_LIB}
    gdi32
    user32
    opengl32
    dwrite
    ole32
    winmm
    "${FFMPEG_DIR}/lib/avcodec.lib"
    "${FFMPEG_DIR}/lib/avformat.lib"
    "${FFMPEG_DIR}/lib/avutil.lib"
    "${FFMPEG_DIR}/lib/swresample.lib"
    "${FFMPEG_DIR}/lib/swscale.lib"
    "${WEBVIEW2_SDK_DIR}/build/native/x64/WebView2LoaderStatic.lib"
)

set(FFMPEG_RUNTIME_DLLS
    "${FFMPEG_DIR}/bin/avcodec-62.dll"
    "${FFMPEG_DIR}/bin/avformat-62.dll"
    "${FFMPEG_DIR}/bin/avutil-60.dll"
    "${FFMPEG_DIR}/bin/swresample-6.dll"
    "${FFMPEG_DIR}/bin/swscale-9.dll")
foreach(FFMPEG_DLL IN LISTS FFMPEG_RUNTIME_DLLS)
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${FFMPEG_DLL}" "$<TARGET_FILE_DIR:main>")
endforeach()
