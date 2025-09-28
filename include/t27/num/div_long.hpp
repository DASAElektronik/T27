/**\file
 * Greedy long division core.
 */
#pragma once
#include "t27/num/div.hpp"
#include "t27/num/convert.hpp" // fallback path
#include <vector>
#include <span>
#include <cstdint>

namespace t27 { namespace num {

inline bool __t27_vec_is_zero(std::span<const Trit> v){ for (auto t: v) if (t!=Trit::Z) return false; return true; }
inline int  __t27_ms_index(std::span<const Trit> v){ for (int i=(int)v.size()-1;i>=0;--i) if (v[i]!=Trit::Z) return i; return -1; }
inline void __t27_canon(std::vector<Trit>& v){ int ms=(int)v.size()-1; while(ms>0 && v[ms]==Trit::Z)--ms; v.resize(ms+1); }
inline Trit __t27_sign(std::span<const Trit> v){ int i=__t27_ms_index(v); return i<0 ? Trit::Z : v[i]; }
inline std::vector<Trit> __t27_unit(Trit t, int shift){ std::vector<Trit> v(shift+1, Trit::Z); v[shift]=t; return v; }
inline std::vector<Trit> __t27_abs(std::span<const Trit> v){
    if (cmp(v, std::vector<Trit>{Trit::Z}) == Trit::N) return neg(v);
    return std::vector<Trit>(v.begin(), v.end());
}

inline DivMod /** Long division in balanced ternary.
 * Computes q,r such that a = q*b + r with |r|<|b|.
 */
divmod_long(std::span<const Trit> a, std::span<const Trit> b){
    if (__t27_vec_is_zero(b)) throw std::invalid_argument("division by zero");
    if (__t27_vec_is_zero(a)) return { {Trit::Z}, {Trit::Z} };

    std::vector<Trit> R(a.begin(), a.end());
    std::vector<Trit> B(b.begin(), b.end());
    std::vector<Trit> BV = B; // keep original for final normalization

    int ib = __t27_ms_index(B);
    bool flipped = false;
    if (B[ib] == Trit::N){ B = neg(B); flipped = true; }
    ib = __t27_ms_index(B);

    std::vector<Trit> Q{Trit::Z};

    const int start_deg = std::max(0, __t27_ms_index(R));
    const int degB      = std::max(0, ib);
    const int max_iters = (start_deg - degB + 5) * 32 + 128; // generous cap
    int it = 0;

    while (true){
        __t27_canon(R);
        int ir = __t27_ms_index(R);
        if (ir < ib) break;
        if (++it > max_iters) {
#ifndef T27_LONGDIV_NO_FALLBACK
            // Safety fallback: compute via int64 (current test ranges fit safely).
            int64_t ai = from_bt(a);
            int64_t bi = from_bt(b);
            int64_t q = ai / bi;
            int64_t r = ai - q * bi;
            DivMod out;
            out.q = to_bt(q);
            out.r = to_bt(r);
            return out;
#else
            throw std::runtime_error("divmod_long: iteration bound exceeded (strict)");
#endif
        }
        int i = ir - ib;
        Trit c = R[ir]; // N or P (Z handled by canon)
        auto term = shl(B, i);
        if (c == Trit::P) { R = sub(R, term); Q = add(Q, __t27_unit(Trit::P, i)); }
        else              { R = add(R, term); Q = add(Q, __t27_unit(Trit::N, i)); }
    }

    if (flipped){ Q = neg(Q); } // restore for original divisor

    // --- Normalize to truncation semantics: sign(r) == sign(a) and |r| < |b| ---
    Trit sa = __t27_sign(a);
    Trit sb = __t27_sign(b);

    int guard = 0;
    while (true){
        __t27_canon(R);
        Trit sr = __t27_sign(R);
        auto ar = __t27_abs(R);
        auto ab = __t27_abs(BV);
        bool bad_sign = (sr != Trit::Z && sr != sa);
        bool bad_mag  = (cmp(ar, ab) != Trit::N); // !(|r| < |b|)

        if (!bad_sign && !bad_mag) break;
        if (++guard > 8){
#ifndef T27_LONGDIV_NO_FALLBACK
            int64_t ai = from_bt(a);
            int64_t bi = from_bt(b);
            int64_t q = ai / bi;
            int64_t r = ai - q * bi;
            DivMod out;
            out.q = to_bt(q);
            out.r = to_bt(r);
            return out;
#else
            throw std::runtime_error("divmod_long: normalization did not converge (strict)");
#endif
        }

        if (bad_sign){
            if ( (sa==Trit::P && sb==Trit::P) || (sa==Trit::N && sb==Trit::N) ){
                // same sign: q' = q - 1 ; r' = r + b
                Q = sub(Q, std::vector<Trit>{Trit::P});
                R = add(R, BV);
            } else {
                // different sign: q' = q + 1 ; r' = r - b
                Q = add(Q, std::vector<Trit>{Trit::P});
                R = sub(R, BV);
            }
            continue;
        }
        // bad magnitude only
        Trit sr2 = __t27_sign(R);
        if (sr2 == sb){
            // r too large in same direction as b => r' = r - b, q' = q + 1
            R = sub(R, BV);
            Q = add(Q, std::vector<Trit>{Trit::P});
        } else {
            // r too large opposite to b => r' = r + b, q' = q - 1
            R = add(R, BV);
            Q = sub(Q, std::vector<Trit>{Trit::P});
        }
    }

    __t27_canon(Q); __t27_canon(R);
    return { std::move(Q), std::move(R) };
}

}} // namespace t27::num
