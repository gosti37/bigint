#ifndef BIGINT_HPP
#define BIGINT_HPP

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <cstring>
#include <cstdint>
#include <string>
#include <ostream>
#include <vector>

namespace bigint
{
    class BigInt
    {
        private:
            std::vector<uint32_t> limbs;
            bool is_negative;
        public:
            inline BigInt() : limbs(1, 0), is_negative(false) {};

            inline BigInt(const char *str) : limbs(1, 0), is_negative(false)
            {
                std::string s = str;
                if (s.empty()) throw std::invalid_argument("Can not take an empty string as initializer");

                if (s[0] == '-')
                {
                    s.erase(0, 1);
                    is_negative = true;
                }

                if (s.empty()) throw std::invalid_argument("Can not take an empty string as initializer");
                
                for (const auto& c : s)
                {
                    if (c < '0' || c > '9')
                        throw std::invalid_argument("Can not take a non-digit value as initializer");

                    auto carry = static_cast<uint64_t>(c - '0');
                    for (auto& l : limbs)
                    {
                        uint64_t cur = static_cast<uint64_t>(l) * 10 + carry;
                        l = static_cast<uint32_t>(cur);
                        carry = cur >> 32;
                    }

                    if (carry) limbs.push_back(static_cast<uint32_t>(carry));
                }
                if (limbs.size() == 1 && limbs.at(0) == 0)
                    is_negative = false;
            }

            template <typename T, typename = typename std::enable_if_t<std::is_integral_v<T>>>
            inline BigInt(T num) : is_negative(false)
            {
                unsigned long long m;

                if constexpr (std::is_signed_v<T>)
                {
                    if (num < 0)
                    {
                        is_negative = true;
                        m = 0ull - static_cast<unsigned long long>(num);
                    }
                    else m = static_cast<unsigned long long>(num);
                }
                else m = num;

                do { limbs.push_back(static_cast<uint32_t>(m & 0xFFFFFFFFull)); m >>= 32; }
                while (m != 0);

                if (limbs.size() == 1 && limbs.at(0) == 0)
                    is_negative = false;
            }

            template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
            explicit inline BigInt(T num)
            {
                if (!std::isfinite(num)) throw std::domain_error("Can not take a NaN or inf value as initializer");

                num = std::trunc(num);

                if (num == 0) { *this = BigInt(0); return; }

                bool neg = num < 0;
                num = std::fabs(num);

                int exp;
                T mant = std::frexp(num, &exp);

                constexpr int bits = std::numeric_limits<T>::digits;

                auto m = static_cast<unsigned long long>(std::ldexp(mant, bits));

                int shift = exp - bits;

                if (shift < 0) m >>= -shift;
                
                limbs.clear();

                do { limbs.push_back(static_cast<uint32_t>(m & 0xFFFFFFFFull)); m >>= 32; }
                while (m != 0);

                if (shift > 0)
                {
                    int whole = shift / 32, part = shift % 32;

                    if (part)
                    {
                        uint32_t carry = 0;
                        for (auto& l : limbs)
                        {
                            uint32_t next = l >> (32 - part);
                            l = (l << part) | carry;
                            carry = next;
                        }

                        if (carry) limbs.push_back(carry);
                    }

                    limbs.insert(limbs.begin(), whole, 0);
                }

                is_negative = neg;
            }

            template <typename T, typename = typename std::enable_if_t<std::is_integral_v<T>>>
            inline T to_int() const
            {
                if (limbs.size() > 2) throw std::overflow_error("Integer overflow");

                unsigned long long mag = limbs.at(0);

                if (limbs.size() == 2) mag |= static_cast<unsigned long long>(limbs.at(1)) << 32;

                constexpr auto maxv = static_cast<unsigned long long>(std::numeric_limits<T>::max());

                if constexpr (std::is_signed_v<T>)
                {
                    if (is_negative)
                    {
                        if (mag > maxv + 1)
                            throw std::overflow_error("Integer overflow");
                        return static_cast<T>(0ull - mag);
                    }
                }

                else if (is_negative && mag != 0)
                    throw std::overflow_error("Assigning negative value into an unsigned type");

                if (mag > maxv) throw std::overflow_error("Integer overflow");

                return static_cast<T>(mag);
            }

            inline std::string to_string() const
            {
                if (limbs.empty()) return "0";

                auto tmp = limbs;

                std::string out;

                do
                {
                    uint64_t rem = 0;

                    for (size_t i = tmp.size(); i-- > 0; )
                    {
                        uint64_t cur = (rem << 32) | tmp.at(i);
                        tmp.at(i) = static_cast<uint32_t>(cur / 10);
                        rem = cur % 10;
                    }

                    out.push_back(static_cast<char>('0' + rem));

                    while (tmp.size() > 1 && tmp.back() == 0) tmp.pop_back();
                }
                while (!(tmp.size() == 1 && tmp.at(0) == 0));

                if (is_negative) out.push_back('-');

                std::reverse(out.begin(), out.end());

                return out;
            }

            friend inline void trim(BigInt& num);
            friend inline BigInt absBigInt(const BigInt& num);
            friend inline int compareBigInt(const BigInt& num1, const BigInt& num2);
            friend inline BigInt addBigInt(const BigInt& num1, const BigInt& num2);
            friend inline BigInt subBigInt(const BigInt& num1, const BigInt& num2);
            friend inline BigInt mulBigInt(const BigInt& num1, const BigInt& num2);
            friend inline std::pair<BigInt, BigInt> divBigInt(const BigInt& num1, const BigInt& num2);
            friend inline BigInt operator|(const BigInt& num1, const BigInt& num2);
            friend inline BigInt operator<<(const BigInt& num, const BigInt& bits);
            friend inline BigInt operator>>(const BigInt& num, const BigInt& bits);
            friend inline std::ostream& operator<<(std::ostream& os, const BigInt& num);



            BigInt& operator+=(const BigInt& num);
            BigInt& operator++(void);
            BigInt operator++(int);
            BigInt& operator-=(const BigInt& num);
            BigInt& operator--(void);
            BigInt operator--(int);
            BigInt& operator*=(const BigInt& num);
            BigInt& operator/=(const BigInt& num);
            BigInt& operator%=(const BigInt& num);
            BigInt& operator|=(const BigInt& num);
            BigInt& operator<<=(const BigInt& bits);
            BigInt& operator>>=(const BigInt& bits);
    };

    inline void trim(BigInt& num)
    {
        while (num.limbs.size() > 1 && num.limbs.back() == 0)
        {
            num.limbs.pop_back();
        }
        if (num.limbs.empty()) num.limbs.push_back(0);
    }

    inline BigInt absBigInt(const BigInt& num)
    {
        auto abs = num;

        abs.is_negative = false;

        return abs;
    }

    inline int compareBigInt(const BigInt& num1, const BigInt& num2)
    {
        if (num1.is_negative && num2.is_negative)
        {
            auto n1 = num1.limbs;
            auto n2 = num2.limbs;

            if (n1.size() != n2.size())
            {
                return n1.size() < n2.size() ? 1 : -1;
            }

            for (size_t i = num1.limbs.size(); i-- > 0; )
            {
                if (num1.limbs.at(i) != num2.limbs.at(i))
                {
                    return n1.at(i) < n2.at(i) ? 1 : -1;
                }
            }
        }

        if (num1.is_negative && !num2.is_negative) return -1;

        if (!num1.is_negative && num2.is_negative) return 1;

        if (!num1.is_negative && !num2.is_negative)
        {
            if (num1.limbs.size() != num2.limbs.size())
            {
                return num1.limbs.size() > num2.limbs.size() ? 1 : -1;
            }

            for (size_t i = num1.limbs.size(); i-- > 0; )
            {
                if (num1.limbs.at(i) != num2.limbs.at(i))
                {
                    return num1.limbs.at(i) > num2.limbs.at(i) ? 1 : -1;
                }
            }

            return 0;
        }

        return 0;
    }

    inline BigInt addBigInt(const BigInt& num1, const BigInt& num2)
    {
        if (!num1.is_negative != !num2.is_negative)
        {
            BigInt res;
            BigInt n1;
            BigInt n2;

            switch (compareBigInt(absBigInt(num1), absBigInt(num2)))
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

            res.is_negative = n1.is_negative;

            size_t maxSize = n1.limbs.size() > n2.limbs.size() ? n1.limbs.size() : n2.limbs.size();
            res.limbs.resize(n1.limbs.size());

            int borrow = 0;

            for (size_t i = 0; i < maxSize; i++)
            {
                int64_t sum = static_cast<int64_t>(n1.limbs.at(i)) - (i < n2.limbs.size() ? static_cast<int64_t>(n2.limbs.at(i)) : 0) - borrow;
                if (sum < 0)
                {
                    sum += 4294967296LL;
                    borrow = 1;
                }
                else borrow = 0;
                res.limbs.at(i) = static_cast<uint32_t>(sum);
            }

            trim(res);

            if (res.limbs.size() == 1 && res.limbs.at(0) == 0)
                res.is_negative = false;

            return res;
        }
        else
        {
            BigInt res;
            auto n1 = num1;
            auto n2 = num2;

            if (n1.is_negative && n2.is_negative) res.is_negative = true;

            size_t maxSize = n1.limbs.size() > n2.limbs.size() ? n1.limbs.size() : n2.limbs.size();
            res.limbs.resize(maxSize + 1);

            uint64_t carry = 0;
            size_t i = 0;

            for (i = 0; i < maxSize || carry; i++)
            {
                uint64_t sum = carry + 
                        (i < n1.limbs.size() ? n1.limbs.at(i) : 0) + 
                        (i < n2.limbs.size() ? n2.limbs.at(i) : 0);

                res.limbs.at(i) = static_cast<uint32_t>(sum);
                carry = sum >> 32;
            }

            trim(res);

            if (res.limbs.size() == 1 && res.limbs.at(0) == 0)
                res.is_negative = false;

            return res;
        }
    }
    
    inline BigInt subBigInt(const BigInt& num1, const BigInt& num2)
    {
        if (!num1.is_negative != !num2.is_negative)
        {
            BigInt res;
            auto n1 = num1;
            auto n2 = num2;
            if (n1.is_negative) res.is_negative = true;

            size_t maxSize = n1.limbs.size() > n2.limbs.size() ? n1.limbs.size() : n2.limbs.size();
            res.limbs.resize(maxSize + 1);

            uint64_t carry = 0;
            size_t i = 0;

            for (i = 0; i < maxSize || carry; i++)
            {
                uint64_t sum = carry + (i < n1.limbs.size() ? n1.limbs.at(i) : 0) + (i < n2.limbs.size() ? n2.limbs.at(i) : 0);
                res.limbs.at(i) = static_cast<uint32_t>(sum);
                carry = sum >> 32;
            }

            trim(res);

            if (res.limbs.size() == 1 && res.limbs.at(0) == 0)
                res.is_negative = false;

            return res;
        }

        else
        {
            BigInt res;
            BigInt n1;
            BigInt n2;

            switch (compareBigInt(absBigInt(num1), absBigInt(num2)))
            {
                case 1:
                    n1 = num1;
                    n2 = num2;
                    res.is_negative = num1.is_negative;
                    break;
                case -1:
                    n1 = num2;
                    n2 = num1;
                    res.is_negative = !num2.is_negative;
                    break;
                default:
                    return res;
            }

            size_t maxSize = num1.limbs.size() > num2.limbs.size() ? num1.limbs.size() : num2.limbs.size();
            res.limbs.resize(n1.limbs.size());

            int borrow = 0;

            for (size_t i = 0; i < maxSize; i++)
            {
                int64_t sum = static_cast<int64_t>(n1.limbs.at(i)) - (i < n2.limbs.size() ? static_cast<int64_t>(n2.limbs.at(i)) : 0) - borrow;
                if (sum < 0)
                {
                    sum += 4294967296LL;
                    borrow = 1;
                }
                else borrow = 0;
                res.limbs.at(i) = static_cast<uint32_t>(sum);
            }

            trim(res);

            if (res.limbs.size() == 1 && res.limbs.at(0) == 0)
                res.is_negative = false;

            return res;
        }
    }

    inline BigInt mulBigInt(const BigInt& num1, const BigInt& num2)
    {
        BigInt res;

        res.limbs.assign(num1.limbs.size() + num2.limbs.size(), 0);

        for (size_t i = 0; i < num1.limbs.size(); i++)
        {
            uint64_t carry = 0;

            for (size_t j = 0; j < num2.limbs.size(); j++)
            {
                size_t idx = i + j;

                uint64_t cur = static_cast<uint64_t>(res.limbs.at(idx))
                             + static_cast<uint64_t>(num1.limbs.at(i)) * num2.limbs.at(j)
                             + carry;

                res.limbs.at(idx) = static_cast<uint32_t>(cur);
                carry = cur >> 32;
            }

            res.limbs.at(i + num2.limbs.size()) += static_cast<uint32_t>(carry);
        }

        trim(res);

        res.is_negative = (num1.is_negative != num2.is_negative);
        if (res.limbs.size() == 1 && res.limbs.at(0) == 0)
            res.is_negative = false;

        return res;
    }
    
    inline std::pair<BigInt, BigInt> divBigInt(const BigInt& num1, const BigInt& num2)
    {
        if (num2.limbs.at(0) == 0 && num2.limbs.size() == 1) throw std::domain_error("Can't divide by zero");

        BigInt divisor = num2;
        divisor.is_negative = false;

        BigInt quotient;
        quotient.limbs.assign(num1.limbs.size(), 0);

        if (!num1.is_negative != !num2.is_negative) quotient.is_negative = true;
        if (num1.is_negative && num2.is_negative) quotient.is_negative = false;

        BigInt remainder;

        for (size_t i = num1.limbs.size(); i-- > 0; )
        {
            remainder.limbs.insert(remainder.limbs.begin(), num1.limbs.at(i));
            trim(remainder);

            uint32_t q = 0;

            if (compareBigInt(remainder, divisor) >= 0)
            {
                uint32_t low = 0;
                uint32_t high = 0xFFFFFFFFu;

                while (low < high)
                {
                    uint32_t mid = low + (high - low) / 2 + 1;

                    if (compareBigInt(mulBigInt(divisor, BigInt(mid)), remainder) <= 0)
                        low = mid;
                    else
                     high = mid - 1;
                }

                q = low;

                remainder = subBigInt(remainder, mulBigInt(divisor, BigInt(q)));
            }

            quotient.limbs.at(i) = q;
        }

        trim(quotient);
        trim(remainder);

        quotient.is_negative = (num1.is_negative != num2.is_negative);
        if (quotient.limbs.size() == 1 && quotient.limbs.at(0) == 0)
            quotient.is_negative = false;
        remainder.is_negative = num1.is_negative;
        if (remainder.limbs.size() == 1 && remainder.limbs.at(0) == 0)
            remainder.is_negative = false;

        return {quotient, remainder};
    }

    /*------------------------------------------------------------*/
                            /*Overloads*/
    /*------------------------------------------------------------*/

    inline BigInt operator+(const BigInt& num1, const BigInt& num2)
    {
        return addBigInt(num1, num2);
    }
    inline BigInt& BigInt::operator+=(const BigInt& num)
    {
        *this = addBigInt(*this, num);

        return *this;
    }
    inline BigInt& BigInt::operator++()
    {
        return *this += 1;
    }
    inline BigInt BigInt::operator++(int)
    {
        BigInt old_int = *this;
        ++(*this);
        return old_int;
    }


    inline BigInt operator-(const BigInt& num1, const BigInt& num2)
    {
        return subBigInt(num1, num2);
    }
    inline BigInt& BigInt::operator-=(const BigInt& num)
    {
        *this = subBigInt(*this, num);

        return *this;
    }
    inline BigInt& BigInt::operator--()
    {
        return *this -= 1;
    }
    inline BigInt BigInt::operator--(int)
    {
        BigInt old_int = *this;
        --(*this);
        return old_int;
    }


    inline BigInt operator*(const BigInt& num1, const BigInt& num2)
    {
        return mulBigInt(num1, num2);
    }
    inline BigInt& BigInt::operator*=(const BigInt &num)
    {
        *this = mulBigInt(*this, num);

        return *this;
    }


    inline BigInt operator/(const BigInt& num1, const BigInt& num2)
    {
        return divBigInt(num1, num2).first;
    }
    inline BigInt& BigInt::operator/=(const BigInt& num)
    {
        *this = divBigInt(*this, num).first;

        return *this;
    }


    inline BigInt operator%(const BigInt& num1, const BigInt& num2)
    {
        return divBigInt(num1, num2).second;
    }
    inline BigInt& BigInt::operator%=(const BigInt &num)
    {
        *this = *this % num;

        return *this;
    }


    inline BigInt operator|(const BigInt& num1, const BigInt& num2)
    {
        BigInt res;
        size_t maxSize = (num1.limbs.size() > num2.limbs.size()) ? num1.limbs.size() : num2.limbs.size();
        res.limbs.resize(maxSize);

        for (size_t i = 0; i < maxSize; i++)
        {
            res.limbs.at(i) = (num1.limbs.size() > i ? num1.limbs.at(i) : 0) | (num2.limbs.size() > i ? num2.limbs.at(i) : 0);
        }

        trim(res);
        
        return res;
    }



    inline BigInt operator<<(const BigInt& num, const BigInt& bits)
    {
        if (bits.is_negative) throw std::invalid_argument("Negative shifting bit count");

        const size_t shift = bits.to_int<size_t>();
        const size_t wordShift = shift / 32;
        const unsigned bitShift = shift % 32;

        BigInt res;
        res.limbs.assign(num.limbs.size() + wordShift + 1, 0);

        for (size_t i = 0; i < num.limbs.size(); i++)
        {
            uint64_t v = static_cast<uint64_t>(num.limbs.at(i)) << bitShift;

            res.limbs.at(i + wordShift) |= static_cast<uint32_t>(v);
            res.limbs.at(i + wordShift + 1) |= static_cast<uint32_t>(v >> 32);
        }

        trim(res);

        res.is_negative = num.is_negative;
        if (res.limbs.size() == 1 && res.limbs.at(0) == 0)
            res.is_negative = false;

        return res;
    }
    inline BigInt& BigInt::operator<<=(const BigInt& bits)
    {
        *this = *this << bits;

        return *this;
    }
    
    inline BigInt operator>>(const BigInt& num, const BigInt& bits)
    {
        if (bits.is_negative) throw std::invalid_argument("Negative shifting bit count");

        const size_t shift = bits.to_int<size_t>();
        const size_t wordShift = shift / 32;
        const unsigned bitShift = shift % 32;

        BigInt res;
        if (wordShift >= num.limbs.size()) return res;

        res.limbs.assign(num.limbs.size() - wordShift + 1, 0);

        for (size_t i = 0; i < num.limbs.size() - wordShift; i++)
        {
            res.limbs.at(i) = num.limbs.at(i + wordShift) >> bitShift;
            if (bitShift && i + wordShift + 1 < num.limbs.size())
                res.limbs.at(i) |= num.limbs.at(i + wordShift + 1) << (32 - bitShift);
        }

        trim(res);

        res.is_negative = num.is_negative;
        if (res.limbs.size() == 1 && res.limbs.at(0) == 0)
            res.is_negative = false;

        return res;
    }
    inline BigInt& BigInt::operator>>=(const BigInt& bits)
    {
        *this = *this >> bits;

        return *this;
    }


    inline std::ostream& operator<<(std::ostream& os, const BigInt& num)
    {
        if (num.limbs.empty()) return os << '0';

        auto tmp = num.limbs;

        std::string out;

        do
        {
            uint64_t rem = 0;

            for (size_t i = tmp.size(); i-- > 0; )
            {
                uint64_t cur = (rem << 32) | tmp.at(i);
                tmp.at(i) = static_cast<uint32_t>(cur / 10);
                rem = cur % 10;
            }

            out.push_back(static_cast<char>('0' + rem));

            while (tmp.size() > 1 && tmp.back() == 0) tmp.pop_back();
        }
        while (!(tmp.size() == 1 && tmp.at(0) == 0));

        if (num.is_negative) out.push_back('-');

        std::reverse(out.begin(), out.end());

        return os << out;
    }


    inline bool operator==(const BigInt& num1, const BigInt& num2)
    {
        if (compareBigInt(num1, num2) == 0) return true;
        else return false;
    }

    inline bool operator!=(const BigInt& num1, const BigInt& num2)
    {
        if (!(num1 == num2)) return true;
        else return false;
    }

    inline bool operator<(const BigInt& num1, const BigInt& num2)
    {
        if (compareBigInt(num1, num2) == -1) return true;
        else return false;
    }

    inline bool operator>(const BigInt& num1, const BigInt& num2)
    {
        if (!(num1 < num2) && !(num1 == num2)) return true;
        else return false;
    }

    inline bool operator<=(const BigInt& num1, const BigInt& num2)
    {
        if (compareBigInt(num1, num2) == -1 || compareBigInt(num1, num2) == 0) return true;
        else return false;
    }

    inline bool operator>=(const BigInt& num1, const BigInt& num2)
    {
        if (compareBigInt(num1, num2) == 1 || compareBigInt(num1, num2) == 0) return true;
        else return false;
    }
}

#endif //BIGINT_HPP