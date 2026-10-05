# SPDX-License-Identifier: MIT
find_package(Doxygen REQUIRED)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile")
  add_custom_target(docs
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/build/doxygen
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} VERBATIM)
else()
  set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs/_build")
  set(DOXYGEN_GENERATE_HTML YES)
  doxygen_add_docs(docs include src docs WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
