# =============================================================================
# ---------------------------- Globals definitions ----------------------------
# =============================================================================
# Modern CMake for module of Cubix
# [J. Dudouet]
#

# Here is the project name
project (CubixTools LANGUAGES CXX)
#
print(STATUS " -> ${PROJECT_NAME}")

# =============================================================================
# ----------------------------- Files definitions -----------------------------
# =============================================================================

file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
list(REMOVE_ITEM headers ${CMAKE_CURRENT_SOURCE_DIR}/LinkDef.h)

if(ROOT_FOUND)
    include(GenerateROOTLibrary)
    GENERATE_ROOT_DICTIONARY(${PROJECT_NAME} LINKDEF LinkDef.h HEADERS ${headers} SOURCES ${sources} DEPENDENCIES cubix::CubixCore ROOT::Hist ROOT::Gui tkn::tkphysics)
else ()
    add_library(${PROJECT_NAME} SHARED ${sources})
endif()

add_library(cubix::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# directories for includes
target_include_directories( ${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# define public headers to be installed
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME   ${PROJECT_NAME}
    PUBLIC_HEADER "${headers}"
    CXX_EXTENSIONS OFF
    BUILD_WITH_INSTALL_NAME_DIR 1
    BUILD_WITH_INSTALL_RPATH 1
    INSTALL_NAME_DIR "@rpath"
    )

target_compile_features(${PROJECT_NAME}
    PUBLIC cxx_constexpr
    PUBLIC cxx_std_11
    )

# =============================================================================
# -------------------------- Install local targets ----------------------------
# =============================================================================

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    PUBLIC_HEADER DESTINATION include
    )
