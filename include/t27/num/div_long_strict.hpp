/** \cond INTERNAL */
/**\file
 * Strict long division (debug/verification).
 */
#pragma once
#include "t27/num/div.hpp"
#include <vector>
#include <span>

namespace t27 { namespace num {

inline bool __v0_vec_is_zero(std::span<const Trit> v){ for (auto t: v) if (t!=Trit::Z) return false; return true; }
inline int  __v0_ms_index(std::span<const Trit> v){ for (int i=(int)v.size()-1;i>=0;--i) if (v[i]!=Trit::Z) return i; return -1; }
inline void __v0_canon(std::vector<Trit>& v){ int ms=(int)v.size()-1; while(ms>0 && v[ms]==Trit::Z)--ms; v.resize(ms+1); }
inline Trit __v0_sign(std::span<const Trit> v){ int i=__v0_ms_index(v); return i<0 ? Trit::Z : v[i]; }
inline std::vector<Trit> __v0_unit(Trit t, int shift){ std::vector<Trit> v(shift+1, Trit::Z); v[shift]=t; return v; }

// Strict pure version (no fallback). Similar greedy scheme, larger caps, throws on anomalies.
inline DivMod /** Strict variant of long division (no fallback). Throws on anomalies. */
divmod_long_strict(std::span<const Trit> a, std::span<const Trit> b){
    if (__v0_vec_is_zero(b)) throw std::invalid_argument("division by zero");
    if (__v0_vec_is_zero(a)) return { {Trit::Z}, {Trit::Z} };

    std::vector<Trit> R(a.begin(), a.end());
    std::vector<Trit> B(b.begin(), b.end());
    std::vector<Trit> BV = B;

    int ib = __v0_ms_index(B);
    bool flipped = false;
    if (B[ib] == Trit::N){ B = neg(B); flipped = true; }
    ib = __v0_ms_index(B);

    std::vector<Trit> Q{Trit::Z};

    const int start_deg = std::max(0, __v0_ms_index(R));
    const int degB      = std::max(0, ib);
    const int max_iters = (start_deg - degB + 5) * 64 + 256;
    int it = 0;

    while (true){
        __v0_canon(R);
        int ir = __v0_ms_index(R);
        if (ir < ib) break;
        if (++it > max_iters) throw std::runtime_error("divmod_long_strict: iteration bound exceeded");
        int i = ir - ib;
        Trit c = R[ir];
        auto term = shl(B, i);
        if (c == Trit::P) { R = sub(R, term); Q = add(Q, __v0_unit(Trit::P, i)); }
        else              { R = add(R, term); Q = add(Q, __v0_unit(Trit::N, i)); }
    }

    if (flipped){ Q = neg(Q); }

    // trunc-toward-zero normalization
    Trit sa = __v0_sign(a);
    Trit sb = __v0_sign(b);
    int guard = 0;
    while (true){
        __v0_canon(R);
        Trit sr = __v0_sign(R);
        bool bad_sign = (sr != Trit::Z && sr != sa);
        // magnitude check via cmp(|r|,|b|) using sign logic
        auto Ar = (sr==Trit::N) ? neg(R) : R;
        auto Ab = (sb==Trit::N) ? neg(BV): BV;
        bool bad_mag = (cmp(Ar, Ab) != Trit::N);
        if (!bad_sign && !bad_mag) break;
        if (++guard > 16) throw std::runtime_error("divmod_long_strict: normalization failed");
        if (bad_sign){
            if ( (sa==Trit::P && sb==Trit::P) || (sa==Trit::N && sb==Trit::N) ){
                Q = sub(Q, std::vector<Trit>{Trit::P});
                R = add(R, BV);
            } else {
                Q = add(Q, std::vector<Trit>{Trit::P});
                R = sub(R, BV);
            }
            continue;
        }
        if (__v0_sign(R) == sb){
            R = sub(R, BV);
            Q = add(Q, std::vector<Trit>{Trit::P});
        } else {
            R = add(R, BV);
            Q = sub(Q, std::vector<Trit>{Trit::P});
        }
    }

    __v0_canon(Q); __v0_canon(R);
    return { std::move(Q), std::move(R) };
}

}} // namespace t27::num

/** \endcond */
