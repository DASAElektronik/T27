#pragma once
/**
 * @file div3k.hpp
 * @brief Division by powers of three (3^k) with balanced remainder.
 */
#include "ops.hpp"
#include "fixed.hpp"
#include <utility>

namespace t27::num {

struct DivMod3kBalanced {
    std::vector<Trit> q; ///< nearest-integer quotient (drop k LSB trits)
    std::vector<Trit> r; ///< balanced remainder in [-(3^k-1)/2 .. + (3^k-1)/2]
};

/** Divide by 3^k with balanced remainder (dynamic width). */
inline DivMod3kBalanced divmod3k_balanced(std::span<const Trit> x, int k) {
    if (k <= 0) return { std::vector<Trit>(x.begin(), x.end()), {Trit::Z} };
    if (x.empty()) return { {Trit::Z}, {Trit::Z} };
    auto q = lshr(x, k);
    std::vector<Trit> r;
    int take = std::min<int>(k, (int)x.size());
    r.reserve(take);
    for (int i = 0; i < take; ++i) r.push_back(x[i]);
    int ms = (int)r.size() - 1;
    while (ms > 0 && r[ms] == Trit::Z) --ms;
    r.resize(ms + 1);
    return { std::move(q), std::move(r) };
}

struct DivMod3kBalanced27 {
    Tword27 q;
    Tword27 r;
    bool inexact{false}; ///< true iff r != 0
};

/** Divide a 27T word by 3^k; sets inexact if any dropped LSB trits were non-zero. */
inline DivMod3kBalanced27 divmod3k_balanced27(const Tword27& a, int k) {
    DivMod3kBalanced27 out{};
    if (k <= 0) { out.q = a; out.r = Tword27{}; out.inexact = false; return out; }
    if (k >= Tword27::width) {
        out.q = Tword27{};
        out.r = a;
        for (auto t : a.t) if (t != Trit::Z) { out.inexact = true; break; }
        return out;
    }
    out.q = lshr27(a, k).first;
    Tword27 r{};
    for (int i = 0; i < k; ++i) r.t[i] = a.t[i];
    for (int i = 0; i < k; ++i) if (a.t[i] != Trit::Z) { out.inexact = true; break; }
    out.r = r;
    return out;
}
} // namespace t27::num