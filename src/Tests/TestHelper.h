#pragma once

#include <stdexcept>
#include <string>
#include <iostream>

#define TRY_TEST(x) try { if (x() != 0) throw std::runtime_error("Did not return 0"); std::cout << "Test " << #x << " PASSED" << std::endl; } catch (std::exception& e) { std::cout << "Test " << #x << " failed: " << e.what() << std::endl; return -1; }

#define TEST_ASSERT(x) if (!(x)) throw std::runtime_error("Assertion failed: " #x)