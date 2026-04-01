# FindOpenImageIO_Safe.cmake
# A fallback that manually creates the OpenImageIO::OpenImageIO imported target
# from known include/library paths, in case the installed CMake package config
# is broken (e.g. missing tool executables like iconvert, oiiotool, etc.)

find_path(OpenImageIO_INCLUDE_DIR OpenImageIO/imageio.h
    PATHS /usr/include /usr/local/include
    NO_DEFAULT_PATH
)

find_library(OpenImageIO_LIBRARY
    NAMES OpenImageIO
    PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
    NO_DEFAULT_PATH
)

find_library(OpenImageIO_Util_LIBRARY
    NAMES OpenImageIO_Util
    PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
    NO_DEFAULT_PATH
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenImageIO_Safe
    REQUIRED_VARS OpenImageIO_INCLUDE_DIR OpenImageIO_LIBRARY
)

if(OpenImageIO_Safe_FOUND)
    if(NOT TARGET OpenImageIO::OpenImageIO)
        add_library(OpenImageIO::OpenImageIO SHARED IMPORTED)
        set_target_properties(OpenImageIO::OpenImageIO PROPERTIES
            IMPORTED_LOCATION "${OpenImageIO_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OpenImageIO_INCLUDE_DIR}"
        )
        if(OpenImageIO_Util_LIBRARY)
            add_library(OpenImageIO::OpenImageIO_Util SHARED IMPORTED)
            set_target_properties(OpenImageIO::OpenImageIO_Util PROPERTIES
                IMPORTED_LOCATION "${OpenImageIO_Util_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${OpenImageIO_INCLUDE_DIR}"
            )
            set_target_properties(OpenImageIO::OpenImageIO PROPERTIES
                INTERFACE_LINK_LIBRARIES "OpenImageIO::OpenImageIO_Util"
            )
        endif()
    endif()
endif()
