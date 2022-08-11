# We assume this is add_subdirectory-ed into
# 
# We are only defining tests in this file

# Only enable tests if static build (dlls cause problems with shared build)

if (NOT BUILD_SHARED_LIBS)
	# Create test source list into TestsSrcs using "MyTests.cpp" as the driver file
	create_test_sourcelist(TestSrcs
		BasicTestRunner.cpp # Driver file
		BasicTests.cpp
	)

	# Add test executable
	add_executable(BasicTestRunner ${TestSrcs})

	message("Test sources: ${TestSrcs}")

	set_target_properties(BasicTestRunner
		PROPERTIES
			CXX_STANDARD 17
			CXX_STANDARD_REQUIRED YES
			CXX_EXTENSIONS NO # Turning Extensions off for increased compatibility - see https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
			) 

	target_compile_definitions(BasicTestRunner PUBLIC CPP_ENGINE_TESTS=1)

	target_include_directories(BasicTestRunner PRIVATE $<TARGET_PROPERTY:cpp_engine_lib,INCLUDE_DIRECTORIES>)
	target_link_libraries(BasicTestRunner PRIVATE cpp_engine_lib)

	# Go through test sources and add tests
	set(TestsToRun ${TestSrcs})
	remove(TestsToRun BasicTestRunner.cpp)

	foreach (test ${TestsToRun})
		get_filename_component(test_name ${test} NAME_WE)
		add_test(NAME ${test_name} COMMAND BasicTestRunner ${test_name})
	#	set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_PATH_VAR}")
	endforeach()
endif()