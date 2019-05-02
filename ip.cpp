#include "ip.h"

namespace updater {

ip::ip(std::string dotted_decimal)
{
    std::vector<unsigned char> decimals = {};

    std::string delimiter = ".";
    size_t pos = 0;

    while ((pos = dotted_decimal.find(delimiter)) != std::string::npos)
    {
        std::string token = dotted_decimal.substr(0, pos);

        unsigned int decimal = std::stoul(token);
        decimals.push_back(decimal);

        dotted_decimal.erase(0, pos + delimiter.length());
    }

    unsigned int d = std::stoul(dotted_decimal);
    decimals.push_back(d);

    if (decimals.size() != 4) throw new std::exception();

    _a = decimals[0];
    _b = decimals[1];
    _c = decimals[2];
    _d = decimals[3];
}

ip::ip(const unsigned char &a, const unsigned char &b, const unsigned char &c, const unsigned char &d)
{
    _a = a;
    _b = b;
    _c = c;
    _d = d;
}

std::string ip::toString() const
{
    unsigned int a = static_cast<unsigned>(_a);
    unsigned int b = static_cast<unsigned>(_b);
    unsigned int c = static_cast<unsigned>(_c);
    unsigned int d = static_cast<unsigned>(_d);

    std::string ip = "";

    ip.append(std::to_string(a));
    ip.append(".");
    ip.append(std::to_string(b));
    ip.append(".");
    ip.append(std::to_string(c));
    ip.append(".");
    ip.append(std::to_string(d));

    return ip;
}

std::vector<unsigned char> ip::getDecimals() const
{
    return std::vector<unsigned char> {_a, _b, _c, _d};
}

bool ip::operator==(const ip &other) const
{
    bool a = _a == other._a;
    bool b = _b == other._b;
    bool c = _c == other._c;
    bool d = _d == other._d;

    if (a && b && c && d) return true;

    return false;
}

}
