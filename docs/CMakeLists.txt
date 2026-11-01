# Documentation site: the Doxygen API reference, embedded in the pages built
# by Zensical. The output goes to docs/site/; see docs/README.md.
find_package(Doxygen REQUIRED)
find_program(ZENSICAL_EXECUTABLE zensical REQUIRED)

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
