# We assume this is add_subdirectory-ed into
# 
# We are only defining tests in this file

# Add test executable
add_executable(DrawingTests
	"Geometry.cpp"
)

set_target_properties(DrawingTests
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO # Turning Extensions off for increased compatibility - see https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
		$<EC_PROFILE:EXE_LINKER_FLAGS ${CMAKE_LINKER_FLAGS} /Profile>
		) 

target_compile_definitions(DrawingTests PUBLIC CPP_ENGINE_TESTS=1)

target_include_directories(DrawingTests PRIVATE $<TARGET_PROPERTY:cpp_engine_lib,INCLUDE_DIRECTORIES>)
target_link_libraries(DrawingTests PRIVATE cpp_engine_lib)
target_link_libraries(DrawingTests PRIVATE GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(DrawingTests)

