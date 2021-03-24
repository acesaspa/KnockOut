#include "Source.h"
#include "stb_image.h"
#include "PowerUp.h"
#include <iostream>

void Source::passValue(PowerUp& a) {
	std::cout << "passValue\n";
	a.foo();
}