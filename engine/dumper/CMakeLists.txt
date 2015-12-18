cmake_minimum_required(VERSION 2.8)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(ROOT_SRC "${CMAKE_CURRENT_SOURCE_DIR}")

set(PATH_SRC_DEX_FILE "${ROOT_SRC}/dex_file.cc")
set(PATH_SRC_DUMPER "${ROOT_SRC}/dumper.cc")
set(PATH_SRC_SPEW "${ROOT_SRC}/../../util/spew.cc")

set(PATH_INC_UTIL "${ROOT_SRC}/../../util/")

set(PATH_OUT "${ROOT_SRC}/../../bin")

set(LIB_NAME "dex")

set(LIB_TYPE "SHARED")

set(TGE_LIB "DEX")

set(EXE_NAME "dumper")

set(TGE_EXE "DUMPER")


include_directories(${PATH_INC_UTIL})

#add_library(${TGE_LIB} ${LIB_TYPE}
#           ${PATH_SRC_DEX_FILE})

#set_target_properties(${TGE_LIB} PROPERTIES
#    LIBRARY_OUTPUT_DIRECTORY ${PATH_OUT}
#    OUTPUT_NAME ${LIB_NAME}
#)

add_executable(${TGE_EXE}
               ${PATH_SRC_SPEW}
               ${PATH_SRC_DEX_FILE}
               ${PATH_SRC_DUMPER})

set_target_properties(${TGE_EXE} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PATH_OUT}
    OUTPUT_NAME ${EXE_NAME}
)