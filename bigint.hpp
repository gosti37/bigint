#ifndef BIGINT_HPP
#define BIGINT_HPP

#include <ostream>
#if defined(_WIN32) || defined(_WIN64)
    #define NOMINMAX
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#include <stdexcept>
#include <cstring>
#include <string>
#include <iostream>

namespace bigint
{
    class BigInt
    {
        public:
            std::string digits;

            inline BigInt() : digits(1, 0) {};

            inline BigInt(const char *str)
            {
                digits.resize(strlen(str));
                for (size_t i = 0; i < digits.size(); i++)
                {
                    digits.at(i) = str[digits.size() - 1 - i] - '0';
                }

                while (digits.size() > 1 && digits.back() == 0)
                {
                    digits.pop_back();
                }
                if (digits.empty()) digits.push_back(0);
            }

            BigInt& operator+=(BigInt& num);
            BigInt& operator+=(long long num);
            BigInt& operator-=(BigInt& num);
            BigInt& operator-=(long long num);
            BigInt& operator*=(BigInt& num);
            BigInt& operator*=(long long num);
            BigInt& operator/=(BigInt& num);
            BigInt& operator/=(long long num);
    };

    inline void trim(BigInt& num)
    {
        while (num.digits.size() > 1 && num.digits.back() == 0)
        {
            num.digits.pop_back();
        }
        if (num.digits.empty()) num.digits.push_back(0);
    }

    inline void printBigInt(const BigInt& num)
    {
        std::string out;
        out.reserve(num.digits.size() + 1);
        for (size_t i = num.digits.size(); i-- > 0; )
        {
            out += static_cast<char>('0' + num.digits.at(i));
        }

        out += '\n';

        #if defined (_WIN32) || defined (_WIN64)
            DWORD written;
            WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), out.data(),
                                    static_cast<DWORD>(out.size()), &written, NULL);
        #else
            write(1, out.data(), out.size());
        #endif
    }

    /*------------------------------------------------------------*/
                        /*Arithmetic Operations*/
    /*------------------------------------------------------------*/

    inline int compareBigInt(const BigInt& num1, const BigInt& num2)
    {
        if (num1.digits.size() != num2.digits.size())
        {
            return num1.digits.size() > num2.digits.size() ? 1 : -1;
        }

        for (size_t i = num1.digits.size(); i-- > 0; )
        {
            if (num1.digits.at(i) != num2.digits.at(i))
            {
                return num1.digits.at(i) > num2.digits.at(i) ? 1 : -1;
            }
        }

        return 0;
    }

    inline BigInt addBigInt(const BigInt& num1, const BigInt& num2)
    {
        BigInt res;
        size_t maxSize = num1.digits.size() > num2.digits.size() ? num1.digits.size() : num2.digits.size();
        res.digits.resize(maxSize + 1);

        size_t carry = 0;
        size_t i = 0;

        for (i = 0; i < maxSize || carry; i++)
        {
            int sum = carry + (i < num1.digits.size() ? num1.digits.at(i) : 0) + (i < num2.digits.size() ? num2.digits.at(i) : 0);
            res.digits.at(i) = sum % 10;
            carry = sum / 10;
        }

        trim(res);

        return res;
    }
    
    inline BigInt subBigInt(const BigInt& num1, const BigInt& num2)
    {
        BigInt res;
        BigInt n1;
        BigInt n2;

        switch (compareBigInt(num1, num2))
        {
            case 1:
                n1 = num1;
                n2 = num2;
                break;
            case -1:
                n1 = num2;
                n2 = num1;
                break;
            default:
                return res;
        }

        size_t maxSize = num1.digits.size() > num2.digits.size() ? num1.digits.size() : num2.digits.size();
        res.digits.resize(n1.digits.size());

        int borrow = 0;

        for (size_t i = 0; i < maxSize; i++)
        {
            int sum = n1.digits.at(i) - (i < n2.digits.size() ? n2.digits.at(i) : 0) - borrow;
            if (sum < 0)
            {
                sum += 10;
                borrow = 1;
            }
            else borrow = 0;
            res.digits.at(i) = sum % 10;
        }

        trim(res);

        return res;
    }

    inline BigInt mulBigInt(const BigInt& num1, const BigInt& num2)
    {
        BigInt res;
        res.digits.resize(num1.digits.size() + num2.digits.size());

        for (size_t i = 0; i < num1.digits.size(); i++)
        {
            for (size_t j = 0; j < num2.digits.size(); j++)
            {
                size_t idx = i+j;
                res.digits.at(idx) += num1.digits.at(i) * num2.digits.at(j);
                res.digits.at(idx + 1) += res.digits.at(idx) / 10;
                res.digits.at(idx) %= 10;
            }
        }

        trim(res);

        return res;
    }
    
    inline BigInt divBigInt(const BigInt& num1, const BigInt& num2)
    {
        if (num2.digits == "0" && num2.digits.size() == 1) throw std::domain_error("Can't divide by zero");

        BigInt quotient;
        quotient.digits.assign(num1.digits.size(), 0);

        BigInt remainder;

        for (size_t i = num1.digits.size(); i-- > 0; )
        {
            remainder.digits.insert(remainder.digits.begin(), num1.digits.at(i));
            trim(remainder);

            int q = 0;

            while (compareBigInt(remainder, num2) >= 0)
            {
                remainder = subBigInt(remainder, num2);
                q++;
            }

            quotient.digits.at(i) = static_cast<char>(q);
        }

        trim(quotient);

        return quotient;
    }

    /*------------------------------------------------------------*/
                            /*Overloads*/
    /*------------------------------------------------------------*/

    inline BigInt operator+(const BigInt& num1, const BigInt& num2)
    {
        return addBigInt(num1, num2);
    }
    inline BigInt operator+(const BigInt& num1, const long long num2)
    {
        BigInt bignum(std::to_string(num2).c_str());

        return addBigInt(num1, bignum);
    }
    inline BigInt operator+(const long long num1, const BigInt& num2)
    {
        BigInt bignum(std::to_string(num1).c_str());

        return addBigInt(bignum, num2);
    }
    inline BigInt& BigInt::operator+=(BigInt& num)
    {
        *this = addBigInt(*this, num);

        return *this;
    }
    inline BigInt& BigInt::operator+=(long long num)
    {
        BigInt bignum(std::to_string(num).c_str());

        *this = addBigInt(*this, bignum);

        return *this;
    }



    inline BigInt operator-(const BigInt& num1, const BigInt& num2)
    {
        return subBigInt(num1, num2);
    }
    inline BigInt operator-(const BigInt& num1, long long num2)
    {
        BigInt bignum(std::to_string(num2).c_str());

        return subBigInt(num1, bignum);
    }
    inline BigInt operator-(long long num1, const BigInt& num2)
    {
        BigInt bignum(std::to_string(num1).c_str());

        return subBigInt(bignum, num2);
    }
    inline BigInt& BigInt::operator-=(BigInt& num)
    {
        *this = subBigInt(*this, num);

        return *this;
    }
    inline BigInt& BigInt::operator-=(long long num)
    {
        BigInt bignum(std::to_string(num).c_str());

        *this = subBigInt(*this, bignum);

        return *this;
    }


    inline BigInt operator*(const BigInt& num1, const BigInt& num2)
    {
        return mulBigInt(num1, num2);
    }
    inline BigInt operator*(const BigInt& num1, long long num2)
    {
        BigInt bignum(std::to_string(num2).c_str());
        return mulBigInt(num1, bignum);
    }
    inline BigInt operator*(long long num1, const BigInt& num2)
    {
        BigInt bignum(std::to_string(num1).c_str());
        return mulBigInt(bignum, num2);
    }
    inline BigInt& BigInt::operator*=(BigInt &num)
    {
        *this = mulBigInt(*this, num);

        return *this;
    }
    inline BigInt& BigInt::operator*=(long long num)
    {
        BigInt bignum(std::to_string(num).c_str());

        *this = mulBigInt(*this, bignum);

        return *this;
    }


    inline BigInt operator/(const BigInt& num1, const BigInt& num2)
    {
        return divBigInt(num1, num2);
    }
    inline BigInt operator/(const BigInt& num1, long long num2)
    {
        BigInt bignum(std::to_string(num2).c_str());
        return divBigInt(num1, bignum);
    }
    inline BigInt operator/(long long num1, const BigInt& num2)
    {
        BigInt bignum(std::to_string(num1).c_str());
        return divBigInt(bignum, num2);
    }
    inline BigInt& BigInt::operator/=(BigInt &num)
    {
        *this = divBigInt(*this, num);

        return *this;
    }
    inline BigInt& BigInt::operator/=(long long num)
    {
        BigInt bignum(std::to_string(num).c_str());

        *this = divBigInt(*this, bignum);

        return *this;
    }


    inline std::ostream& operator<<(std::ostream& os, const BigInt& num)
    {
        std::string out;
        out.reserve(num.digits.size() + 1);
        for (size_t i = num.digits.size(); i-- > 0; )
        {
            out += static_cast<char>('0' + num.digits.at(i));
        }

        return os << out;
    }
}

#endif //BIGINT_HPP