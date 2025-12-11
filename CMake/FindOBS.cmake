# - Try to find OBS
# Once done this will define
#  OBS_FOUND - System has OBS
#  OBS_INCLUDE_DIRS - The OBS include directories
#  OBS_LIBRARIES - The libraries needed to use OBS
#  OBS_DEFINITIONS - Compiler switches required for using OBS

# Find the path to the OBS installation
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_lib_suffix 64)
else()
    set(_lib_suffix 32)
endif()

# Common paths for OBS installation
if(WIN32)
    set(_obs_include_path "$ENV{OBS_INSTALL_PATH}/include")
    set(_obs_lib_path "$ENV{OBS_INSTALL_PATH}/lib")
elseif(APPLE)
    set(_obs_include_path "/usr/local/include/obs")
    set(_obs_lib_path "/usr/local/lib")
else() # Linux
    set(_obs_include_path "/usr/include/obs")
    set(_obs_lib_path "/usr/lib${_lib_suffix}")
endif()

# Find the include directories
find_path(OBS_INCLUDE_DIR
    NAMES obs.h
    PATHS ${_obs_include_path}
    PATH_SUFFIXES obs
)

# Find the obs library
find_library(OBS_LIBRARY
    NAMES obs
    PATHS ${_obs_lib_path}
)

# Find the obs-frontend-api library
find_library(OBS_FRONTEND_API_LIBRARY
    NAMES obs-frontend-api
    PATHS ${_obs_lib_path}
)

# Set include directories
set(OBS_INCLUDE_DIRS ${OBS_INCLUDE_DIR})

# Set libraries
set(OBS_LIBRARIES ${OBS_LIBRARY} ${OBS_FRONTEND_API_LIBRARY})

# Handle the QUIETLY and REQUIRED arguments and set OBS_FOUND to TRUE
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OBS DEFAULT_MSG
    OBS_LIBRARY OBS_FRONTEND_API_LIBRARY OBS_INCLUDE_DIR)

mark_as_advanced(OBS_INCLUDE_DIR OBS_LIBRARY OBS_FRONTEND_API_LIBRARY)
