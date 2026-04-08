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

add_executable(Medify ${EXEC_SRCS})

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
