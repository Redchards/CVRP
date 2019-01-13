#ifndef STRING_UTILS_HXX
#define STRING_UTILS_HXX

#include <algorithm> 
#include <cctype>
#include <locale>

namespace Utils
{

// trim from start (in place)
static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
}

static inline bool is_prefix(const std::string& prefix, std::string& str) {
    auto res = std::mismatch(prefix.begin(), prefix.end(), str.begin());
    return res.first == prefix.end();
}

}

#endif // STRING_UTILS_HXX