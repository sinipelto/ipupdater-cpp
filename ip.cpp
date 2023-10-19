#include "ip.hh"

namespace updater
{
    /// @brief Default constructor.
    /// Initializes to zero ip.
    ip::ip()
    {
        _a = 0;
        _b = 0;
        _c = 0;
        _d = 0;
    }

    ip::ip(std::string dotted_decimal)
    {
        // TODO: check input: IP regex match

        std::vector<unsigned char> decimals = {};

        const std::string delimiter = ".";
        size_t pos = 0;

        unsigned d;

        while ((pos = dotted_decimal.find(delimiter)) != std::string::npos)
        {
            std::string token = dotted_decimal.substr(0, pos);
            d = std::stoul(token);

            decimals.push_back(d);

            dotted_decimal.erase(0, pos + delimiter.length());
        }

        d = std::stoul(dotted_decimal);
        decimals.push_back(d);

        if (decimals.size() != 4)
            throw new std::exception;

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

    const std::string ip::toString() const
    {
        unsigned int a = static_cast<unsigned>(_a);
        unsigned int b = static_cast<unsigned>(_b);
        unsigned int c = static_cast<unsigned>(_c);
        unsigned int d = static_cast<unsigned>(_d);

        std::string ip = "";

        ip += std::to_string(a);
        ip += ".";
        ip += std::to_string(b);
        ip += ".";
        ip += std::to_string(c);
        ip += ".";
        ip += std::to_string(d);

        return ip;
    }

    const std::vector<unsigned char> ip::getDecimals() const
    {
        return std::vector<unsigned char>{_a, _b, _c, _d};
    }

    bool ip::operator==(const ip &other) const
    {
        return _a == other._a &&
               _b == other._b &&
               _c == other._c &&
               _d == other._d;
    }
}
