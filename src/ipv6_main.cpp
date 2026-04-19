/**
* Copyright © 2017 Stefan Hallas Mulvad
* ipv6 subnet calculator
*/

#include "ipv6calc.h"

int main(int argc, char* argv[])
{
	ipv6calc::Ipv6Calculator calculator;
	return calculator.run(argc, argv);
}
