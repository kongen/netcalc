/**
* Copyright © 2017 Stefan Hallas Mulvad
* ipv4 subnet calculator
*/

#include "ipv4calc.h"

int main(int argc, char* argv[])
{
	ipv4calc::Ipv4Calculator calculator;
	return calculator.run(argc, argv);
}
