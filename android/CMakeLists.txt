cmake_minimum_required(VERSION 2.8)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(ROOT_SRC "${CMAKE_CURRENT_SOURCE_DIR}")

set(PATH_INC "${ROOT_SRC}/runtime/")

set(PATH_OUT "${ROOT_SRC}/../bin")

set(PATH_SRC_UTF "${ROOT_SRC}/runtime/utf.cc")
set(PATH_SRC_DEX_FILE "${ROOT_SRC}/runtime/dex_file.cc")


set(NAME_OUT "dex")

set(LIB_TYPE "SHARED")

set(TGE "DEX")


include_directories(${PATH_INC})
add_library(${TGE} ${LIB_TYPE}
            ${PATH_SRC_UTF}
            ${PATH_SRC_DEX_FILE})


set_target_properties(${TGE} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PATH_OUT}
    OUTPUT_NAME ${NAME_OUT}
)