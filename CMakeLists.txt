
# =============================================================================
# ---------------------------- Globals definitions ----------------------------
# =============================================================================

# Modern CMake for module of tkn-submodule-example-example
# [J. Dudouet]
#
# Effective cmake required
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(tkn-submodule-example
    DESCRIPTION  "CMake project for the tkn-submodule-example project"
    LANGUAGES CXX
    )

# ==================================================================
# ------------------- Add the TkN submodule ------------------------
# ==================================================================

SET(DB_DOWNLOAD ON CACHE BOOL "Download the TkN database at install")
add_subdirectory(tkn-lib)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/tkn-lib)

if(USE_ROOT)
    find_package(ROOT 6.20 CONFIG)
endif()

# ==================================================================
# ----------------- Compile exec without ROOT ----------------------
# ==================================================================
add_executable(my_prog ${CMAKE_CURRENT_SOURCE_DIR}/src/my_prog.cpp)
target_link_libraries(my_prog PUBLIC tkn::tkphysics)
install(PROGRAMS ${CMAKE_BINARY_DIR}/my_prog DESTINATION bin)

# ==================================================================
# ------------------- Compile exec with ROOT  ----------------------
# ==================================================================

if(USE_ROOT)
    add_executable(my_prog_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src/my_prog_ROOT.cpp)
    target_link_libraries(my_prog_ROOT PUBLIC tkn::tkphysics ROOT::Hist)
    install(PROGRAMS ${CMAKE_BINARY_DIR}/my_prog_ROOT DESTINATION bin)
endif()
