# We assume this is add_subdirectory-ed into
# 
# We are only defining tests in this file

# Only enable tests if static build (dlls cause problems with shared build)

if (NOT BUILD_SHARED_LIBS)
	
	# Add test executable
	add_executable(BasicTestRunner 
		"BasicTests.cpp"
	)

	set_target_properties(BasicTestRunner
		PROPERTIES
			CXX_STANDARD 17
			CXX_STANDARD_REQUIRED YES
			CXX_EXTENSIONS NO # Turning Extensions off for increased compatibility - see https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
			) 

	target_compile_definitions(BasicTestRunner PUBLIC CPP_ENGINE_TESTS=1)

	target_include_directories(BasicTestRunner PRIVATE $<TARGET_PROPERTY:cpp_engine_lib,INCLUDE_DIRECTORIES>)
	target_link_libraries(BasicTestRunner PRIVATE cpp_engine_lib)
	target_link_libraries(BasicTestRunner PRIVATE GTest::gtest_main)

	

	add_custom_command( TARGET BasicTestRunner POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                        $<TARGET_RUNTIME_DLLS:BasicTestRunner>
                        $<TARGET_FILE_DIR:BasicTestRunner>
                    COMMAND_EXPAND_LISTS)

	# Go through test sources and add tests
	set(TestsToRun ${TestSrcs})
	remove(TestsToRun BasicTestRunner.cpp)

	include(GoogleTest)
    gtest_discover_tests(BasicTestRunner)
endif()