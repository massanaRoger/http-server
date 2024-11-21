#pragma once

#include "server.h"
#include <iostream>

GET("/hello", []() {
	std::cout << "Hello, world!" << std::endl;
});

GET("/bye", []() {
	std::cout << "Bye" << std::endl;
});
