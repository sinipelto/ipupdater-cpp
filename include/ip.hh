#ifndef IP_H
#define IP_H

#include <string>
#include <list>
#include <exception>
#include <vector>

namespace updater
{
    class ip
    {
    public:
        ip();
        ip(std::string dotted_decimal);
        ip(const unsigned char &a, const unsigned char &b, const unsigned char &c, const unsigned char &d);
        const std::string toString() const;
        const std::vector<unsigned char> getDecimals() const;
        bool operator==(const ip &other) const;

    private:
        unsigned char _a;
        unsigned char _b;
        unsigned char _c;
        unsigned char _d;
    };
}

#endif // IP_H
