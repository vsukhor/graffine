# Documentation site: the Doxygen API reference, embedded in the pages built
# by Zensical. The output goes to docs/site/; see docs/README.md.
#
# Missing tools disable the `docs` target instead of failing the configure
# step, so library and test builds are never blocked by them. Zensical is
# also looked up in docs/.venv (see docs/README.md), with no need to activate it.
find_package(Doxygen)
find_program(ZENSICAL_EXECUTABLE zensical
    HINTS ${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin
)

if(NOT DOXYGEN_FOUND OR NOT ZENSICAL_EXECUTABLE)
    set(_missing)
    if(NOT DOXYGEN_FOUND)
        list(APPEND _missing "Doxygen")
    endif()
    if(NOT ZENSICAL_EXECUTABLE)
        list(APPEND _missing "Zensical")
    endif()
    list(JOIN _missing " and " _missing)
    message(WARNING
        "BUILD_DOCS is ON, but ${_missing} not found: the 'docs' target is "
        "disabled. See docs/README.md for installing the documentation tools.")
    return()
endif()

file(GLOB_RECURSE GRAFFINE_PUBLIC_HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)
file(GLOB_RECURSE GRAFFINE_DOC_PAGES ${CMAKE_CURRENT_SOURCE_DIR}/content/*.md)

set(DOXYGEN_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/content/api/reference/index.html)
set(SITE_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/site/index.html)

# Only regenerate the API reference when the Doxyfile or the headers change.
add_custom_command(
    OUTPUT ${DOXYGEN_INDEX_FILE}
    DEPENDS ${GRAFFINE_PUBLIC_HEADERS}
    COMMAND ${CMAKE_COMMAND} -E env GRAFFINE_VERSION=${PROJECT_VERSION}
            ${DOXYGEN_EXECUTABLE} Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
    COMMENT "Generating API reference with Doxygen"
    VERBATIM
)

add_custom_command(
    OUTPUT ${SITE_INDEX_FILE}
    DEPENDS ${DOXYGEN_INDEX_FILE} ${GRAFFINE_DOC_PAGES}
    COMMAND ${ZENSICAL_EXECUTABLE} build --clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/zensical.toml
    COMMENT "Building the documentation site with Zensical"
    VERBATIM
)

add_custom_target(docs ALL DEPENDS ${SITE_INDEX_FILE})
