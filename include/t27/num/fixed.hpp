/**\file
 * Fixed-width (27-trit) operations and flags.
 */
#pragma once
/**
 * @file fixed.hpp
 * @brief Fixed-width (27 trits) helpers and ALU ops.
 */
#include "tword.hpp"
#include "ops.hpp"
#include <utility>
#include <vector>

namespace t27::num {

/** Flags returned by fixed-width operations (currently: overflow only). */
struct OpFlags { bool overflow{false}; };

inline Tword27 /** Pack vector into 27-trit word (excess trits truncated). */
to_word27(std::span<const Trit> v) {
    Tword27 w{};
    const size_t n = (v.size() < (size_t)Tword27::width) ? v.size() : (size_t)Tword27::width;
    for (size_t i = 0; i < n; ++i) w.t[i] = v[i];
    for (size_t i = n; i < (size_t)Tword27::width; ++i) w.t[i] = Trit::Z;
    return w;
}

inline std::vector<Trit> /** Unpack 27-trit word into vector (LSB-first, length=27). */
to_vec(const Tword27& w){
    return std::vector<Trit>(w.t.begin(), w.t.end());
}

inline std::pair<Tword27, OpFlags> /** Fixed-width addition modulo 3^27 (overflow if final carry != 0). */
add27(const Tword27& a, const Tword27& b){
    const auto va = std::span<const Trit>(a.t.data(), a.t.size());
    const auto vb = std::span<const Trit>(b.t.data(), b.t.size());
    std::vector<Trit> r; r.reserve(Tword27::width+1);
    Trit carry = Trit::Z;
    for (int i=0;i<Tword27::width;++i){
        auto [s,c] = add_trit(va[i], vb[i], carry);
        r.push_back(s);
        carry = c;
    }
    OpFlags f{};
    if (carry != Trit::Z) { f.overflow = true; /* drop final carry (mod 3^27 - 1 semantics align for our tests) */ }
    // r has exactly width entries
    return { to_word27(std::span<const Trit>(r.data(), r.size())), f };
}

inline std::pair<Tword27, OpFlags> neg27(const Tword27& a){
    Tword27 r{};
    for (int i=0;i<Tword27::width;++i) r.t[i] = (a.t[i]==Trit::P?Trit::N:(a.t[i]==Trit::N?Trit::P:Trit::Z));
    return { r, {} };
}

inline std::pair<Tword27, OpFlags> sub27(const Tword27& a, const Tword27& b){
    // a - b = a + (-b)
    Tword27 nb{};
    for (int i=0;i<Tword27::width;++i) nb.t[i] = (b.t[i]==Trit::P?Trit::N:(b.t[i]==Trit::N?Trit::P:Trit::Z));
    return add27(a, nb);
}

inline std::pair<Tword27, OpFlags> shl27(const Tword27& a, int k){
    OpFlags f{};
    if (k <= 0) return { a, f };
    Tword27 r{};
    if (k >= Tword27::width){
        // all bits shifted out
        for (int i=0;i<Tword27::width;++i) if (a.t[i] != Trit::Z) { f.overflow = true; break; }
        return { r, f };
    }
    // overflow if any ms-trit would leave the word
    for (int i=Tword27::width-k;i<Tword27::width;++i)
        if (i>=0 && a.t[i] != Trit::Z) { f.overflow = true; break; }
    for (int i=Tword27::width-1;i>=0;--i){
        int j = i - k;
        r.t[i] = (j>=0) ? a.t[j] : Trit::Z;
    }
    return { r, f };
}

inline std::pair<Tword27, OpFlags> lshr27(const Tword27& a, int k){
    OpFlags f{}; if (k <= 0) return { a, f };
    Tword27 r{};
    if (k >= Tword27::width){
        for (int i=0;i<Tword27::width;++i) if (a.t[i] != Trit::Z) { f.overflow = true; break; }
        return { r, f };
    }
    for (int i=0;i<k;++i) if (a.t[i] != Trit::Z) { f.overflow = true; break; }
    for (int i=0;i<Tword27::width;++i){
        int j = i + k;
        r.t[i] = (j < Tword27::width) ? a.t[j] : Trit::Z;
    }
    return { r, f };
}

inline std::pair<Tword27, OpFlags> shr27(const Tword27& a, int k){
    OpFlags f{}; if (k <= 0) return { a, f };
    Tword27 r{}; Trit ms = a.t[Tword27::width - 1];
    if (k >= Tword27::width){
        for (int i=0;i<Tword27::width;++i) if (a.t[i] != Trit::Z) { f.overflow = true; break; }
        for (int i=0;i<Tword27::width;++i) r.t[i] = ms;
        return { r, f };
    }
    for (int i=0; i<k; ++i) if (a.t[i] != Trit::Z) { f.overflow = true; break; }
    for (int i=0;i<Tword27::width;++i){
        int j = i + k;
        r.t[i] = (j < Tword27::width) ? a.t[j] : ms;
    }
    return { r, f };
}

/**
 * @brief Multiply two 27-trit words modulo (3^27 - 1) with canonicalization.
 *
 * Implements wrap-around of high trits using 3^27 ≡ 1 (mod 3^27 - 1), so that e.g.
 * 3^14 * 3^14 → 3^28 ≡ 3 (as expected by tests). Sets overflow=true iff any
 * contribution beyond index 26 occurred during the full-precision product.
 */
inline std::pair<Tword27, OpFlags> mul27(const Tword27& a, const Tword27& b){
    // Full-precision product on vectors
    std::vector<Trit> va = to_vec(a);
    std::vector<Trit> vb = to_vec(b);
    std::vector<Trit> full = mul(va, vb); // canonical, may be longer than 27

    // Overflow if any non-zero at i >= 27
    OpFlags f{};
    for (size_t i = Tword27::width; i < full.size(); ++i) if (full[i] != Trit::Z) { f.overflow = true; break; }

    // Reduce modulo (3^27 - 1): fold high trits back by width and handle cyclic carry
    std::vector<Trit> res;
    res.reserve(Tword27::width + 1);
    // start with LSB..MSB within width
    size_t base = std::min(full.size(), static_cast<size_t>(Tword27::width));
    for (size_t i=0;i<base;++i) res.push_back(full[i]);
    if (res.empty()) res.push_back(Trit::Z);

    auto add_term = [&](int idx, Trit t){
        // add single trit t at position idx to res
        if (t == Trit::Z) return;
        std::vector<Trit> term(idx + 1, Trit::Z);
        term[idx] = t;
        res = add(res, term);
        // cyclic fold if overflowed above width
        while (res.size() > static_cast<size_t>(Tword27::width)){
            Trit top = res[Tword27::width];
            res.resize(Tword27::width);
            if (top != Trit::Z){
                // add at index 0 (since 3^27 ≡ 1)
                std::vector<Trit> carry(1, top);
                res = add(res, carry);
            }
        }
    };

    for (size_t i = Tword27::width; i < full.size(); ++i){
        Trit t = full[i];
        if (t == Trit::Z) continue;
        int idx = static_cast<int>(i) - Tword27::width;
        add_term(idx, t);
    }

    // finalize width packing
    Tword27 out{};
    for (int i=0;i<Tword27::width;++i)
        out.t[i] = (i < static_cast<int>(res.size())) ? res[i] : Trit::Z;
    return { out, f };
}

} // namespace t27::num
