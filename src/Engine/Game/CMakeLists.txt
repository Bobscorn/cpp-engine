# We assume this is add_subdirectory-ed into
# 
# We are only defining tests in this file

# Create test source list into TestsSrcs using "MyTests.cpp" as the driver file
create_test_sourcelist(TestSrcs
	MyTests.cpp
	GameEngine.cpp
)

# Add test executable
add_executable(MyTests ${TestSrcs})

message("Test sources: ${TestSrcs}")

set_target_properties(MyTests
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO # Turning Extensions off for increased compatibility - see https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
		$<EC_PROFILE:EXE_LINKER_FLAGS ${CMAKE_LINKER_FLAGS} /Profile>
		) 

target_compile_definitions(MyTests PUBLIC CPP_ENGINE_TESTS=1)

target_include_directories(MyTests PRIVATE $<TARGET_PROPERTY:cpp_engine_lib,INCLUDE_DIRECTORIES>)

# Go through test sources and add tests
set(TestsToRun ${TestSrcs})
remove(TestsToRun MyTests.cpp)

foreach (test ${TestsToRun})
	get_filename_component(test_name ${test} NAME_WE)
	add_test(NAME ${test_name} COMMAND MyTests ${test_name})
endforeach()