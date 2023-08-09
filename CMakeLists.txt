
# =============================================================================
# ---------------------------- Globals definitions ----------------------------
# =============================================================================
# [J. Dudouet]

# Effective cmake required
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(cubix
        DESCRIPTION  "ROOT based utility for gamma-ray spectroscopy analysis"
        LANGUAGES CXX
        )

# =============================================================================
# ---------------------------- Define color codes -----------------------------
# =============================================================================

if(NOT WIN32)
    string(ASCII 27 Esc)
    set(ColourReset "${Esc}[m")
    set(Red         "${Esc}[31m")
    set(Green       "${Esc}[32m")
    set(Blue        "${Esc}[34m")
    set(Yellow      "${Esc}[33m")
endif()

function(print)
    list(GET ARGV 0 MessageType)
    if(MessageType STREQUAL FATAL_ERROR OR MessageType STREQUAL SEND_ERROR)
        list(REMOVE_AT ARGV 0)
        message("${Red}${ARGV}${ColourReset}")
    elseif(MessageType STREQUAL WARNING)
        list(REMOVE_AT ARGV 0)
        message("${Yellow}WARNING: ${ARGV}${ColourReset}")
    elseif(MessageType STREQUAL NOTICE)
        list(REMOVE_AT ARGV 0)
        message("${Blue}${ARGV}${ColourReset}")
    elseif(MessageType STREQUAL STATUS)
        list(REMOVE_AT ARGV 0)
        message("${Green}${ARGV}${ColourReset}")
    else()
        message("${ARGV}")
    endif()
endfunction()

# ==================================================================
# ---------------------- Define Cubix version ----------------------
# ==================================================================

set(Cubix_VERSION_MAJOR "1")
set(Cubix_VERSION_MINOR "0")
set(Cubix_VERSION "${Cubix_VERSION_MAJOR}.${Cubix_VERSION_MINOR}")

# ==================================================================
# -------------------- Compilation options -------------------------
# ==================================================================

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic")
print(NOTICE "Cubix compiled with c++11 standards")
set(configstd ${CMAKE_CXX${CMAKE_CXX_STANDARD}_STANDARD_COMPILE_OPTION})

# ==================================================================
# ------------------------- Build options --------------------------
# ==================================================================

include(GNUInstallDirs)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

option(BUILTIN_TKN    "build tkn in Cubix"                      OFF)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    print(WARNING "Cubix compiled in ${CMAKE_BUILD_TYPE} mode !")
else()
    print(STATUS "Cubix compiled in ${CMAKE_BUILD_TYPE} mode !")
endif()

# ==================================================================
# ------------------------- Find ROOT ------------------------------
# ==================================================================

find_package(ROOT 6.20 CONFIG REQUIRED)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

include("${ROOT_USE_FILE}")
MESSAGE(STATUS "ROOT Found in $ENV{ROOTSYS}")
MESSAGE(STATUS "ROOT Version: ${ROOT_VERSION} ")

INCLUDE_DIRECTORIES( ${ROOT_INCLUDE_DIR} )
LINK_DIRECTORIES( ${ROOT_LIBRARY_DIR} )

# ==================================================================
# -------------------------- Link TkN ------------------------------
# ==================================================================

if(BUILTIN_TKN)

    find_package(Git REQUIRED)

    set(dir tkn-lib)

    if(NOT EXISTS ${dir}/CMakeLists.txt)
      execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- ${dir}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMAND_ERROR_IS_FATAL ANY)
    endif()

    if(TKN_BRANCH)
        print(WARNING "Using the TkN branch: ${TKN_BRANCH}")
        execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${TKN_BRANCH}
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/${dir}
          COMMAND_ERROR_IS_FATAL ANY)
    endif()

    SET(DB_DOWNLOAD ON CACHE BOOL "Download the TkN database at install")
    add_subdirectory(${dir})
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/tkn-lib)
    include_directories(${dir}/src/physics)
    include_directories(${dir}/src/db)
    include_directories(${dir}/src/base)
else()
    print(NOTICE "Looking for TkN:")
    find_package(tksqlite3 REQUIRED)
    find_package(TkN REQUIRED)
    include_directories(${TKN_INCLUDE_DIR})
endif()

list(APPEND EXTRA_EXTERNAL_LIBRARIES tkn::tkphysics)

# ==================================================================
# -------------------- cubix_config.h definition -------------------
# ==================================================================

# OS type required
if ( UNIX )
    set(CMAKE_OS_TYPE 1)
endif()
if ( WIN32 )
    set(CMAKE_OS_TYPE 2)
endif()
if ( APPLE )
    set(CMAKE_OS_TYPE 3)
endif()

# built the congiguration file
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cubix_config.h.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cubix_config.h"
    )

# to include tkn_config.h
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# ==================================================================
# ------------------- thiscubix definition -------------------------
# ==================================================================

set( CX_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib )
set( CX_BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin )
set( CX_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include )
set( CX_INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )
set( TKN_SYS_DIR ${TKN_INSTALL_DIR} )
if(BUILTIN_TKN)
    set( TKN_SYS_DIR ${CX_INSTALL_DIR} )
endif()

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/thiscubix.sh.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/thiscubix.sh"
    @ONLY
    )
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/thiscubix.csh.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/thiscubix.csh"
    @ONLY
    )
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/cubix-config.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cubix-config"
    @ONLY
    )

# ==================================================================
# --------------------- Define Cubix files -------------------------
# ==================================================================

# set the module path
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# FIX Warning with rpath on macosx
if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

print(NOTICE "Loading libraries")

add_subdirectory (src/core)
include_directories (src/core)

add_subdirectory (src/tools)
include_directories (src/tools)

add_subdirectory (src/radware_interface)
include_directories (src/radware_interface)

add_subdirectory (src/gui)
include_directories (src/gui)

file(GLOB icons ${CMAKE_CURRENT_SOURCE_DIR}/icons/*)
file(GLOB databases ${CMAKE_CURRENT_SOURCE_DIR}/databases)

### ==================================================================
### ------------------- Compile Cubix Program  -----------------------
### ==================================================================

print(NOTICE "Building Cubix executable")

set (user_programs cubix)

foreach( prog ${user_programs} )
    print(STATUS " -> ${prog}")
    add_executable(${prog} ${CMAKE_CURRENT_SOURCE_DIR}/exec/${prog}.cpp)
    target_link_libraries(${prog} ${EXTRA_EXTERNAL_LIBRARIES} ROOT::Rint cubix::CubixGui)
    install(TARGETS ${prog} RUNTIME DESTINATION bin)
endforeach()

### ==================================================================
### ------------------------ Install Targets -------------------------
### ==================================================================

print(NOTICE "Cubix Configuration")

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/thiscubix.sh DESTINATION bin)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/thiscubix.csh DESTINATION bin)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cubix-config DESTINATION bin)
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/cubix-autocompletion.sh DESTINATION bin)

install(FILES ${icons} DESTINATION icons COMPONENT icons)
install(DIRECTORY ${database} DESTINATION ${CMAKE_INSTALL_PREFIX})
