cmake_minimum_required(VERSION 2.8)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(ROOT_SRC "${CMAKE_CURRENT_SOURCE_DIR}")

set(PATH_INC_RUNTIME "${ROOT_SRC}/runtime/")
set(PATH_INC_LIBNATIVEHELPER "${ROOT_SRC}/libnativehelper/include/nativehelper")

set(PATH_OUT "${ROOT_SRC}/../bin")

set(PATH_SRC_UTF "${ROOT_SRC}/runtime/utf.cc")
set(PATH_SRC_STRINGPRINTF "${ROOT_SRC}/runtime/base/stringprintf.cc")
set(PATH_SRC_STRINGPIECE "${ROOT_SRC}/runtime/base/stringpiece.cc")
set(PATH_SRC_DEX_FILE "${ROOT_SRC}/runtime/dex_file.cc")


set(NAME_OUT "dex")

set(LIB_TYPE "SHARED")

set(TGE "DEX")


include_directories(${PATH_INC_LIBNATIVEHELPER}
                    ${PATH_INC_RUNTIME})

add_library(${TGE} ${LIB_TYPE}
            ${PATH_SRC_UTF}
            ${PATH_SRC_STRINGPRINTF}
            ${PATH_SRC_STRINGPIECE}
            ${PATH_SRC_DEX_FILE})


set_target_properties(${TGE} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PATH_OUT}
    OUTPUT_NAME ${NAME_OUT}
)