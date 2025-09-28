#include "t27/num/convert.hpp"
#include <algorithm>
namespace t27::num {
std::vector<Trit> to_bt(int64_t n) {
    std::vector<Trit> out;
    if (n == 0) { out.push_back(Trit::Z); return out; }
    int64_t x = n;
    while (x != 0) {
        int64_t q = x / 3;
        int64_t r = x % 3;
        if (r < 0) { r += 3; q -= 1; }
        if (r == 2) { out.push_back(Trit::N); x = q + 1; }
        else if (r == 1) { out.push_back(Trit::P); x = q; }
        else { out.push_back(Trit::Z); x = q; }
    }
    return out;
}
int64_t from_bt(std::span<const Trit> trits) {
    int64_t p = 1, s = 0;
    for (size_t i = 0; i < trits.size(); ++i) { s += static_cast<int>(trits[i]) * p; p *= 3; }
    return s;
}
std::string to_string(std::span<const Trit> trits) {
    if (trits.empty()) return "0";
    int ms = static_cast<int>(trits.size()) - 1;
    while (ms > 0 && trits[ms] == Trit::Z) --ms;
    std::string s; s.reserve(ms + 1);
    for (int i = ms; i >= 0; --i) s.push_back(to_char(trits[i]));
    return s;
}
std::vector<Trit> parse_bt(std::string_view ms_to_ls) {
    std::vector<Trit> out;
    out.reserve(ms_to_ls.size());
    for (auto it = ms_to_ls.rbegin(); it != ms_to_ls.rend(); ++it) {
        char c = *it;
        if (c == '-' || c == '0' || c == '+') out.push_back(from_char(c));
    }
    if (out.empty()) out.push_back(Trit::Z);
    int ms = static_cast<int>(out.size()) - 1;
    while (ms > 0 && out[ms] == Trit::Z) --ms;
    out.resize(ms + 1);
    return out;
}
} // namespace t27::num