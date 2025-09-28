#include "t27/num/ops.hpp"
#include <algorithm>
namespace t27::num {
static inline void push_canonical(std::vector<Trit>& v, Trit t) { v.push_back(t); }

std::vector<Trit> add(std::span<const Trit> a, std::span<const Trit> b) {
    const size_t n = std::max(a.size(), b.size());
    std::vector<Trit> r; r.reserve(n + 1);
    Trit carry = Trit::Z;
    for (size_t i = 0; i < n; ++i) {
        Trit ta = (i < a.size()) ? a[i] : Trit::Z;
        Trit tb = (i < b.size()) ? b[i] : Trit::Z;
        auto [sum_trit, new_carry] = add_trit(ta, tb, carry);
        push_canonical(r, sum_trit);
        carry = new_carry;
    }
    if (carry != Trit::Z) r.push_back(carry);
    int ms = static_cast<int>(r.size()) - 1;
    while (ms > 0 && r[ms] == Trit::Z) --ms;
    r.resize(ms + 1);
    return r;
}
std::vector<Trit> neg(std::span<const Trit> a) {
    std::vector<Trit> r; r.reserve(a.size());
    for (Trit t : a) r.push_back(t27::num::neg(t));
    int ms = static_cast<int>(r.size()) - 1;
    while (ms > 0 && r[ms] == Trit::Z) --ms;
    r.resize(ms + 1);
    return r;
}
std::vector<Trit> sub(std::span<const Trit> a, std::span<const Trit> b) { return add(a, neg(b)); }
Trit cmp(std::span<const Trit> a, std::span<const Trit> b) {
    int ms = static_cast<int>(std::max(a.size(), b.size())) - 1;
    for (int i = ms; i >= 0; --i) {
        Trit ta = (i < (int)a.size()) ? a[i] : Trit::Z;
        Trit tb = (i < (int)b.size()) ? b[i] : Trit::Z;
        if (ta != tb) return (static_cast<int>(ta) > static_cast<int>(tb)) ? Trit::P : Trit::N;
    }
    return Trit::Z;
}
std::vector<Trit> shl(std::span<const Trit> a, int k) {
    if (k <= 0) return std::vector<Trit>(a.begin(), a.end());
    std::vector<Trit> r; r.reserve(a.size() + k);
    r.insert(r.end(), k, Trit::Z);
    r.insert(r.end(), a.begin(), a.end());
    int ms = static_cast<int>(r.size()) - 1;
    while (ms > 0 && r[ms] == Trit::Z) --ms;
    r.resize(ms + 1);
    return r;
}
std::vector<Trit> shr(std::span<const Trit> a, int k) {
    if (k <= 0) return std::vector<Trit>(a.begin(), a.end());
    if (a.empty()) return {Trit::Z};
    Trit ms_tr = a[a.size() - 1];
    if (k >= (int)a.size()) return { ms_tr };
    std::vector<Trit> r;
    r.insert(r.end(), a.begin() + k, a.end());
    if (ms_tr != Trit::Z) r.push_back(ms_tr);
    int ms = static_cast<int>(r.size()) - 1;
    while (ms > 0 and r[ms] == Trit::Z) --ms;
    r.resize(ms + 1);
    return r;
}
std::vector<Trit> lshr(std::span<const Trit> a, int k) {
    if (k <= 0) return std::vector<Trit>(a.begin(), a.end());
    if (a.empty() || k >= (int)a.size()) return { Trit::Z };
    std::vector<Trit> r;
    r.insert(r.end(), a.begin() + k, a.end());
    int ms = static_cast<int>(r.size()) - 1;
    while (ms > 0 && r[ms] == Trit::Z) --ms;
    r.resize(ms + 1);
    return r;
}
std::vector<Trit> mul(std::span<const Trit> a, std::span<const Trit> b) {
    std::vector<Trit> acc{ Trit::Z };
    for (size_t i = 0; i < b.size(); ++i) {
        Trit tb = b[i];
        if (tb == Trit::Z) continue;
        std::vector<Trit> term = shl(a, static_cast<int>(i));
        if (tb == Trit::N) term = neg(term);
        acc = add(acc, term);
    }
    return acc;
}
} // namespace t27::num