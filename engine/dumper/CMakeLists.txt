cmake_minimum_required(VERSION 2.8)


#==================================================================#
#                The subroutines for specific task                 #
#==================================================================#
# This subroutine builds the dumper part of YADD.
function(SUB_BUILD_PURE_DUMPER)
    set(TYPE_LIB "lib")
    set(TYPE_EXE "exe")

    include_directories(${PATH_INC_DUMPER}
                        ${PATH_INC_UTIL})

    if (BIN_TYPE STREQUAL TYPE_LIB)
        set(LIB_NAME "dexdump")
        set(LIB_TYPE "SHARED")

        add_library(${TGE} ${LIB_TYPE}
                    ${PATH_SRC_UTF}
                    ${PATH_SRC_MISC}
                    ${PATH_SRC_STRINGPIECE}
                    ${PATH_SRC_STRINGPRINTF}
                    ${PATH_SRC_LOG}
                    ${PATH_SRC_DEX_FILE}
                    ${PATH_SRC_DEX_INSTRUCTION}
                    ${PATH_SRC_CMD_OPT}
                    ${PATH_SRC_DUMPER})

        set_target_properties(  ${TGE} PROPERTIES
                                LIBRARY_OUTPUT_DIRECTORY ${PATH_OUT}
                                OUTPUT_NAME ${LIB_NAME})

    elseif (BIN_TYPE STREQUAL TYPE_EXE)
        set(EXE_NAME "dumper")

        add_executable( ${TGE}
                        ${PATH_SRC_UTF}
                        ${PATH_SRC_MISC}
                        ${PATH_SRC_STRINGPIECE}
                        ${PATH_SRC_STRINGPRINTF}
                        ${PATH_SRC_LOG}
                        ${PATH_SRC_DEX_FILE}
                        ${PATH_SRC_DEX_INSTRUCTION}
                        ${PATH_SRC_CMD_OPT}
                        ${PATH_SRC_DUMPER})

        set_target_properties(  ${TGE} PROPERTIES
                                RUNTIME_OUTPUT_DIRECTORY ${PATH_OUT}
                                OUTPUT_NAME ${EXE_NAME})
    else()
        message("Error: BIN_TYPE is not properly specified.")
        return()
    endif()
endfunction()


#==================================================================#
#                    The CMakeLists entry point                    #
#==================================================================#
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Abbreviate the variable
set(ROOT_SRC "${CMAKE_CURRENT_SOURCE_DIR}")

# The paths of to be built source files.
set(PATH_SRC_DEX_FILE           "${ROOT_SRC}/dex_file.cc")
set(PATH_SRC_DEX_INSTRUCTION    "${ROOT_SRC}/dex_instruction.cc")
set(PATH_SRC_DUMPER             "${ROOT_SRC}/dumper.cc")
set(PATH_SRC_SPEW               "${ROOT_SRC}/../../util/spew.cc")
set(PATH_SRC_LOG                "${ROOT_SRC}/../../util/log.cc")
set(PATH_SRC_STRINGPRINTF       "${ROOT_SRC}/../../util/stringprintf.cc")
set(PATH_SRC_STRINGPIECE        "${ROOT_SRC}/../../util/stringpiece.cc")
set(PATH_SRC_MISC               "${ROOT_SRC}/../../util/misc.cc")
set(PATH_SRC_UTF                "${ROOT_SRC}/../../util/utf.cc")
set(PATH_SRC_CMD_OPT            "${ROOT_SRC}/../../util/cmd_opt.cc")

# The header inclusion paths.
set(PATH_INC_UTIL "${ROOT_SRC}/../../util/")
set(PATH_INC_DUMPER "${ROOT_SRC}")

# The binary output path.
set(PATH_OUT "${ROOT_SRC}/../../bin")

# In this POC stage, I prefer pure dumper executable.
set(PURE_DUMPER "true")
set(BIN_TYPE "exe")

set(TGE "YADD")
if (PURE_DUMPER)
    SUB_BUILD_PURE_DUMPER()
else()

endif()

