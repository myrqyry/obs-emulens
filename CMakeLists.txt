cmake_minimum_required(VERSION 3.30...3.32)


# Set project name and version
project(obs-emulens VERSION 1.0.0 LANGUAGES C CXX)

# Set C/C++ standards with appropriate flags for debug/release
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default build type to RelWithDebInfo if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type (default: RelWithDebInfo)" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# Set compiler warnings and optimizations
if(MSVC)
  add_compile_options(/W4 /WX)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(/Od /Zi)
  else()
    add_compile_options(/O2 /Oi /GL)
    add_link_options(/LTCG)
  endif()
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0 -g3 -fno-omit-frame-pointer)
  else()
    add_compile_options(-O3 -march=native -DNDEBUG)
  endif()
  
  if(APPLE)
    add_compile_options(-fvisibility=hidden)
  endif()
  
  if(UNIX AND NOT APPLE)
    add_compile_options(-fPIC)
  endif()
endif()

# Set module path for custom find modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

# Find OBS libraries
find_package(OBS REQUIRED)

if(NOT OBS_FOUND)
    message(FATAL_ERROR "OBS development files not found. Please set OBS_INSTALL_PATH environment variable.")
endif()

# Set plugin installation paths
if(WIN32)
    set(OBS_PLUGIN_DESTINATION "obs-plugins/64bit")
    set(OBS_PLUGIN_DATA_DESTINATION "data/obs-plugins/obs-emulens")
elseif(APPLE)
    set(OBS_PLUGIN_DESTINATION "obs-plugins")
    set(OBS_PLUGIN_DATA_DESTINATION "data/obs/obs-plugins/obs-emulens")
else() # Linux
    set(OBS_PLUGIN_DESTINATION "lib/obs-plugins")
    set(OBS_PLUGIN_DATA_DESTINATION "share/obs/obs-plugins/obs-emulens")
endif()

# Plugin target
add_library(${PROJECT_NAME} MODULE)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

# Add source files
target_sources(${PROJECT_NAME} PRIVATE
    src/plugin-main.c
    src/effects.c
    src/effects.h
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OBS_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OBS_LIBRARIES}
)

# Install the plugin
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION "${OBS_PLUGIN_DESTINATION}"
    RUNTIME DESTINATION "${OBS_PLUGIN_DESTINATION}"
)

# Create data directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/shaders)

# Install shaders
install(DIRECTORY data/
    DESTINATION "${OBS_PLUGIN_DATA_DESTINATION}"
    FILES_MATCHING PATTERN "*.shader"
)

# Copy shaders to build directory
file(GLOB SHADER_FILES "data/shaders/*.shader")
foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    configure_file(
        ${SHADER}
        ${CMAKE_CURRENT_BINARY_DIR}/data/shaders/${SHADER_NAME}
        COPYONLY
    )
endforeach()
