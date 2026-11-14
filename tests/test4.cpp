#include "bigint.hpp"
#include <iostream>

using namespace bigint;

int main(void)
{
    std::cout << "TEST 4 - DIVISIONS\n\n";

    BigInt n1 = "100000000000000000000000000000000000000";
    BigInt n2 = "50000000000000000000000";

    std::cout << "Testing = 'operator/' with n1 and n2:\n";
    std::cout << n1 / n2 << "\n\n";

    std::cout << "Testing = 'operator/' with n1 and long long integer:\n";
    std::cout << n1 / 500000 << "\n\n";

    BigInt n3 = "23333333333333333333333333333333333333333333333333";

    std::cout << "Testing = 'operator/=' with n3 and n2:\n";
    n3 /= n2;
    std::cout << n3 << "\n\n";

    std::cout << "Testing = 'operator/=' with n3 and long long integer:\n";
    n3 /= 23234234234;
    std::cout << n3 << "\n\n";
}