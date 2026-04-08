cmake_minimum_required(VERSION 3.16)
project(Medify VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Pdf REQUIRED)

file(GLOB_RECURSE EXEC_SRCS
    "src/*.cpp"
    "include/*.h"
)

add_executable(Medify MACOSX_BUNDLE ${EXEC_SRCS})

if(APPLE)
    set(APP_ICON_MACOS "${CMAKE_SOURCE_DIR}/assets/Medify.icns")
    set_source_files_properties(${APP_ICON_MACOS} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(Medify PRIVATE ${APP_ICON_MACOS})
endif()

target_include_directories(Medify PRIVATE 
    include 
    include/ui 
    include/tools 
    include/workers
    include/tools/compress_media
    include/tools/pdf_to_image
    include/tools/split_pdf
    include/tools/extract_audio
    include/tools/audio_converter
    include/tools/video_converter
)
target_link_libraries(Medify PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Pdf)

set_target_properties(Medify PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "Medify"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.blendezu.medify"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
)

include(GNUInstallDirs)

install(TARGETS Medify
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_generate_deploy_app_script(
        TARGET Medify
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()

set(CPACK_PACKAGE_NAME "Medify")
set(CPACK_PACKAGE_VENDOR "blendezu")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

if(APPLE)
    set_target_properties(Medify PROPERTIES
        MACOSX_BUNDLE_ICON_FILE "Medify.icns"
    )
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-macOS")
    set(CPACK_DMG_VOLUME_ICON "${CMAKE_SOURCE_DIR}/assets/Medify.icns")
endif()

include(CPack)
