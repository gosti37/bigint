#include "bigint.hpp"
#include <iostream>

using namespace bigint;

int main(void)
{
    std::cout << "TEST 2 - SUBSTRACTIONS\n\n";

    BigInt n1 = "100000000000000000000000000000000000000";
    BigInt n2 = "500000000000000000000000000000000000000";

    std::cout << "Testing = 'operator-' with n1 and n2:\n";
    std::cout << n2 - n1 << "\n\n";

    std::cout << "Testing = 'operator-' with n1 and long long integer:\n";
    std::cout << n1 - 500000 << "\n\n";

    BigInt n3 = "2349827349287349287893487293487928743928734927349";

    std::cout << "Testing = 'operator-=' with n3 and n2:\n";
    n3 -= n2;
    std::cout << n3 << "\n\n";

    std::cout << "Testing = 'operator-=' with n3 and long long integer:\n";
    n3 -= 23234234234;
    std::cout << n3 << "\n\n";
}