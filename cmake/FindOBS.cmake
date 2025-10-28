# - Try to find OBS Studio libraries and headers.
#
# Once done this will define:
#  OBS_FOUND         - System has OBS Studio libraries and headers
#  OBS_INCLUDE_DIRS  - The OBS Studio include directories
#  OBS_LIBRARIES     - The libraries needed to use OBS Studio

find_path(OBS_INCLUDE_DIR obs.h
    PATH_SUFFIXES obs
    PATHS /usr/include /usr/local/include
)

find_library(OBS_LIBRARY obs
    PATHS /usr/lib /usr/local/lib
)

set(OBS_INCLUDE_DIRS ${OBS_INCLUDE_DIR})
set(OBS_LIBRARIES ${OBS_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OBS DEFAULT_MSG
    OBS_LIBRARY OBS_INCLUDE_DIR
)

mark_as_advanced(OBS_INCLUDE_DIR OBS_LIBRARY)
