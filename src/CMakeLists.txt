# TODO add all targets as separate configurations so they can be built all at the same time.
#   editor|template_release|template_debug

configure_file( entry_point.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/entry_point.cpp )

set( SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/entry_point.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/register_types.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/register_types.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/uuid.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/uuid.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils.hpp
        ${FBHEADERS}
        PARENT_SCOPE
)

# Multi-Step .gdextension file configuration.

# First is to load the file into a variable
file( READ gdextension.in GDEXT_FILE_STRING )

# Replace variables with their content
string( CONFIGURE "${GDEXT_FILE_STRING}" GDEXT_FILE_STRING_CONFIGURED)

# use generate so generator expressions are evaluated.
file( GENERATE
        OUTPUT ${GDE_INSTALL_PATH}/${GDE_NAME}.gdextension
        CONTENT "${GDEXT_FILE_STRING_CONFIGURED}"
)
