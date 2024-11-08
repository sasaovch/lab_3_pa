cmake_minimum_required(VERSION 3.23)
project(lab_3_pa C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

add_executable(pa3
    ${CMAKE_CURRENT_SOURCE_DIR}/pa23.c
    ${CMAKE_CURRENT_SOURCE_DIR}/child_work.c
    ${CMAKE_CURRENT_SOURCE_DIR}/parent_work.c
    ${CMAKE_CURRENT_SOURCE_DIR}/work_with_pipes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/bank_robbery.c
    ${CMAKE_CURRENT_SOURCE_DIR}/banking_work.c
    ${CMAKE_CURRENT_SOURCE_DIR}/time_work.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/banking.h
    # ${CMAKE_CURRENT_SOURCE_DIR}/child_work.h
    # ${CMAKE_CURRENT_SOURCE_DIR}/common.h
    # ${CMAKE_CURRENT_SOURCE_DIR}/ipc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pa2345.h
    # ${CMAKE_CURRENT_SOURCE_DIR}/parent_work.h
    # ${CMAKE_CURRENT_SOURCE_DIR}/pipes_const.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pipes_const.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/work_with_pipes.h

)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    set(RUNTIME_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib64 CACHE PATH "Runtime library path")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    set(RUNTIME_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib32 CACHE PATH "Runtime library path")
endif()

find_library(RUNTIME_LIBRARY libruntime.so PATHS ${RUNTIME_LIBRARY_DIR} NO_DEFAULT_PATH)
if(RUNTIME_LIBRARY)
  message(STATUS "Found Runtime library: ${RUNTIME_LIBRARY}")
else(RUNTIME_LIBRARY)
  message(FATAL_ERROR "Runtime library not found (searched ${RUNTIME_LIBRARY_DIR})")
endif(RUNTIME_LIBRARY)

set_property(TARGET pa3
   PROPERTY
   ENVIRONMENT LD_PRELOAD="${RUNTIME_LIBRARY}"
   )

target_link_libraries(pa3 ${RUNTIME_LIBRARY})
