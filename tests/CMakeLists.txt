enable_testing()
set(FRNETLIB_TEST ${FRNETLIB_TEST_STR}_test)
include_directories(${GTEST_INCLUDE_DIRS} ${COMMON_INCLUDES})

file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/tests/*.cpp)
add_executable(${FRNETLIB_TEST} ${TEST_SRC_FILES})
add_dependencies(${FRNETLIB_TEST} googletest)


if(NOT WIN32 OR MINGW)
    target_link_libraries(${FRNETLIB_TEST}
        ${GTEST_LIBS_DIR}/libgtest.a
        ${GTEST_LIBS_DIR}/libgtest_main.a
        )
else()
    target_link_libraries(${FRNETLIB_TEST}
        debug ${GTEST_LIBS_DIR}/DebugLibs/${CMAKE_FIND_LIBRARY_PREFIXES}gtest${CMAKE_FIND_LIBRARY_SUFFIXES}
        optimized ${GTEST_LIBS_DIR}/ReleaseLibs/${CMAKE_FIND_LIBRARY_PREFIXES}gtest${CMAKE_FIND_LIBRARY_SUFFIXES}
        )
    target_link_libraries(${FRNETLIB_TEST}
        debug ${GTEST_LIBS_DIR}/DebugLibs/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main${CMAKE_FIND_LIBRARY_SUFFIXES}
        optimized ${GTEST_LIBS_DIR}/ReleaseLibs/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main${CMAKE_FIND_LIBRARY_SUFFIXES}
        )
endif()

#Link tests
target_link_libraries(${FRNETLIB_TEST} -lgtest -lfrnetlib)
add_test(test1 ${FRNETLIB_TEST})

#Run tests automatically
add_custom_command(TARGET ${FRNETLIB_TEST}
        POST_BUILD
        COMMAND ${PROJECT_SOURCE_DIR}/tests/_test)