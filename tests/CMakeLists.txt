add_subdirectory("${PROJECT_SOURCE_DIR}/lib/vendor/googletest" "lib/vendor/googletest")

# USAGE: package_add_test([test name] [files])
macro(package_add_test TESTNAME)
    # create an exectuable in which the tests will be stored
    message("NAME: ${TESTNAME}")
    message("ARGS: ${ARGN}")
    add_executable(${TESTNAME} ${ARGN})
    # link the Google test infrastructure, mocking library, and a default main fuction to
    # the test executable.  Remove g_test_main if writing your own main function.
    target_link_libraries(${TESTNAME} gtest gmock gtest_main)
    # gtest_discover_tests replaces gtest_add_tests,
    # see https://cmake.org/cmake/help/v3.10/module/GoogleTest.html for more options to pass to it
    gtest_discover_tests(${TESTNAME}
        # set a working directory so your project root so that you can find test data via paths relative to the project root
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

# set(coreFiles
    
# )

package_add_test(Core
    "${PROJECT_SOURCE_DIR}/src/engine/core/Chunk.cpp"
    "${PROJECT_SOURCE_DIR}/src/engine/core/FlatChunkGenerator.cpp"
    "${PROJECT_SOURCE_DIR}/src/engine/core/glm_util.cpp"
    "${PROJECT_SOURCE_DIR}/src/engine/core/LocalChunkHandler.cpp"
    "${PROJECT_SOURCE_DIR}/src/engine/core/types.cpp"
    "${PROJECT_SOURCE_DIR}/src/engine/core/World.cpp"
    "engine/core/Chunk.cpp"
    "engine/core/FlatChunkGenerator.cpp"
    "engine/core/glm_util.cpp"
    "engine/core/LocalChunkHandler.cpp"
    "engine/core/types.cpp"
    "engine/core/World.cpp"
)
# package_add_test(Core_FlatChunkGenerator ${coreFiles})
# package_add_test(Core_glm_util ${coreFiles})
# package_add_test(Core_LocalChunkHandler ${coreFiles})
# package_add_test(Core_types ${coreFiles})
# package_add_test(Core_World ${coreFiles})

# package_add_test(Render_Vertex "${PROJECT_SOURCE_DIR}/src/engine/render/Vertex.cpp")
# package_add_test(Render_VulkanRenderer "${PROJECT_SOURCE_DIR}/src/engine/render/VulkanRenderer.cpp")
# package_add_test(Render_WorldView "${PROJECT_SOURCE_DIR}/src/engine/render/WorldView.cpp")
