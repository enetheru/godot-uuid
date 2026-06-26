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

set( GDEXT_FILE_STRING [[
[configuration]
entry_symbol = "${GDE_NAME}_library_init"
compatibility_minimum = "${GODOT_VERSION_MAJOR}.${GODOT_VERSION_MINOR}"
reloadable = $<IF:$<BOOL:${GODOTCPP_USE_HOT_RELOAD}>,true,false>

[libraries] ]])

foreach( TARGET editor template_debug template_release )
  STRING( JOIN "."  FEATURES "${GODOTCPP_PLATFORM}" "${TARGET}" "${GODOTCPP_ARCH}" )
  STRING( JOIN "\n" GDEXT_FILE_STRING "${GDEXT_FILE_STRING}" "${FEATURES} = \"bin/${GDE_NAME}.${FEATURES}.dll\"" )
endforeach()

# Replace variables with their content
string( CONFIGURE "${GDEXT_FILE_STRING}" GDEXT_FILE_STRING_CONFIGURED)

# use generate so generator expressions are evaluated.
file( GENERATE
        OUTPUT ${GDE_INSTALL_PATH}/${GDE_NAME}.gdextension
        CONTENT "${GDEXT_FILE_STRING_CONFIGURED}"
)
