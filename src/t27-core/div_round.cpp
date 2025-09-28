#include "t27/num/div.hpp"
#include <vector>
namespace t27::num {

static inline std::vector<Trit> abs_vec_in(std::span<const Trit> v){
    if (cmp(v, std::vector<Trit>{Trit::Z}) == Trit::N) return neg(v);
    return std::vector<Trit>(v.begin(), v.end());
}
static inline Trit sgn_in(std::span<const Trit> v){ return cmp(v, std::vector<Trit>{Trit::Z}); }
static inline bool is_zero_in(std::span<const Trit> v){ return sgn_in(v) == Trit::Z; }

DivRR divmod_euclid(std::span<const Trit> a, std::span<const Trit> b){
    if (is_zero_in(b)) throw std::invalid_argument("division by zero");
    auto dm = divmod(a,b);
    if (sgn_in(dm.r) == Trit::N){
        auto q = sub(dm.q, std::vector<Trit>{ sgn_in(b)==Trit::N ? Trit::N : Trit::P });
        auto r = add(dm.r, abs_vec_in(b));
        return { std::move(q), std::move(r) };
    }
    return { std::move(dm.q), std::move(dm.r) };
}
DivRR divmod_floor(std::span<const Trit> a, std::span<const Trit> b){
    if (is_zero_in(b)) throw std::invalid_argument("division by zero");
    auto dm = divmod(a,b);
    Trit sb = sgn_in(b), sr = sgn_in(dm.r);
    bool adjust = (sr != Trit::Z) && (sb != sr);
    if (adjust){
        auto q = sub(dm.q, std::vector<Trit>{Trit::P});
        auto r = add(dm.r, b);
        return { std::move(q), std::move(r) };
    }
    return { std::move(dm.q), std::move(dm.r) };
}
DivRR divmod_ceil(std::span<const Trit> a, std::span<const Trit> b){
    if (is_zero_in(b)) throw std::invalid_argument("division by zero");
    auto dm = divmod(a,b);
    Trit sb = sgn_in(b), sr = sgn_in(dm.r);
    bool adjust = (sr != Trit::Z) && (sb == sr);
    if (adjust){
        auto q = add(dm.q, std::vector<Trit>{Trit::P});
        auto r = sub(dm.r, b);
        return { std::move(q), std::move(r) };
    }
    return { std::move(dm.q), std::move(dm.r) };
}
DivRR divmod_trunc(std::span<const Trit> a, std::span<const Trit> b){
    Trit sa = sgn_in(a), sb = sgn_in(b);
    return (sa == sb) ? divmod_floor(a,b) : divmod_ceil(a,b);
}
static inline bool is_even(std::span<const Trit> v){ size_t nz=0; for (auto t: v) if (t != Trit::Z) ++nz; return (nz%2)==0; }
DivRR divmod_nearest_away(std::span<const Trit> a, std::span<const Trit> b){
    if (is_zero_in(b)) throw std::invalid_argument("division by zero");
    auto dm = divmod(a,b);
    auto ab = abs_vec_in(b);
    auto ar = abs_vec_in(dm.r);
    auto twice = add(ar, ar);
    Trit c2 = cmp(twice, ab);
    if (c2 == Trit::N) return { std::move(dm.q), std::move(dm.r) }; // < half => keep
    // tie or > half => adjust away from zero
    Trit dir = (sgn_in(dm.r) == sgn_in(b)) ? Trit::P : Trit::N; // away from zero
    auto q = (dir==Trit::P) ? add(dm.q, std::vector<Trit>{Trit::P})
                            : sub(dm.q, std::vector<Trit>{Trit::P});
    auto r = (dir==Trit::P) ? sub(dm.r, b) : add(dm.r, b);
    return { std::move(q), std::move(r) };
}
DivRR divmod_nearest_even(std::span<const Trit> a, std::span<const Trit> b){
    if (is_zero_in(b)) throw std::invalid_argument("division by zero");
    auto dm = divmod(a,b);
    auto ab = abs_vec_in(b);
    auto ar = abs_vec_in(dm.r);
    auto twice = add(ar, ar);
    Trit c2 = cmp(twice, ab);
    if (c2 == Trit::N) return { std::move(dm.q), std::move(dm.r) }; // < half
    if (c2 == Trit::Z){ // tie
        if (is_even(dm.q)) return { std::move(dm.q), std::move(dm.r) };
    }
    // > half or tie but q is odd => adjust in tie direction
    Trit dir = (sgn_in(dm.r) == sgn_in(b)) ? Trit::P : Trit::N;
    auto q = (dir==Trit::P) ? add(dm.q, std::vector<Trit>{Trit::P})
                            : sub(dm.q, std::vector<Trit>{Trit::P});
    auto r = (dir==Trit::P) ? sub(dm.r, b) : add(dm.r, b);
    return { std::move(q), std::move(r) };
}

// ---- 27T wrappers ----
static inline DivRR27 pack_rr27(std::span<const Trit> qv, std::span<const Trit> rv){
    DivRR27 out{};
    out.overflow_q = (int)qv.size() > Tword27::width;
    for (int i=0;i<Tword27::width && i<(int)qv.size(); ++i) out.q.t[i]=qv[i];
    for (int i=0;i<Tword27::width && i<(int)rv.size(); ++i) out.r.t[i]=rv[i];
    return out;
}
static inline bool vec_zero(std::span<const Trit> v){ for (auto t: v) if (t != Trit::Z) return false; return true; }

DivRR27 divmod_euclid27(const Tword27& a, const Tword27& b){
    std::vector<Trit> va; va.reserve(Tword27::width);
    std::vector<Trit> vb; vb.reserve(Tword27::width);
    for (int i=0;i<Tword27::width;++i){ va.push_back(a.t[i]); vb.push_back(b.t[i]); }
    DivRR27 out{}; if (vec_zero(vb)) { out.divide_by_zero=true; return out; }
    auto rr = divmod_euclid(va,vb); return pack_rr27(rr.q, rr.r);
}
DivRR27 divmod_floor27(const Tword27& a, const Tword27& b){
    std::vector<Trit> va; va.reserve(Tword27::width);
    std::vector<Trit> vb; vb.reserve(Tword27::width);
    for (int i=0;i<Tword27::width;++i){ va.push_back(a.t[i]); vb.push_back(b.t[i]); }
    DivRR27 out{}; if (vec_zero(vb)) { out.divide_by_zero=true; return out; }
    auto rr = divmod_floor(va,vb); return pack_rr27(rr.q, rr.r);
}
DivRR27 divmod_ceil27(const Tword27& a, const Tword27& b){
    std::vector<Trit> va; va.reserve(Tword27::width);
    std::vector<Trit> vb; vb.reserve(Tword27::width);
    for (int i=0;i<Tword27::width;++i){ va.push_back(a.t[i]); vb.push_back(b.t[i]); }
    DivRR27 out{}; if (vec_zero(vb)) { out.divide_by_zero=true; return out; }
    auto rr = divmod_ceil(va,vb); return pack_rr27(rr.q, rr.r);
}
DivRR27 divmod_trunc27(const Tword27& a, const Tword27& b){
    std::vector<Trit> va; va.reserve(Tword27::width);
    std::vector<Trit> vb; vb.reserve(Tword27::width);
    for (int i=0;i<Tword27::width;++i){ va.push_back(a.t[i]); vb.push_back(b.t[i]); }
    DivRR27 out{}; if (vec_zero(vb)) { out.divide_by_zero=true; return out; }
    auto rr = divmod_trunc(va,vb); return pack_rr27(rr.q, rr.r);
}
DivRR27 divmod_nearest_away27(const Tword27& a, const Tword27& b){
    std::vector<Trit> va; va.reserve(Tword27::width);
    std::vector<Trit> vb; vb.reserve(Tword27::width);
    for (int i=0;i<Tword27::width;++i){ va.push_back(a.t[i]); vb.push_back(b.t[i]); }
    DivRR27 out{}; if (vec_zero(vb)) { out.divide_by_zero=true; return out; }
    auto rr = divmod_nearest_away(va,vb); return pack_rr27(rr.q, rr.r);
}
DivRR27 divmod_nearest_even27(const Tword27& a, const Tword27& b){
    std::vector<Trit> va; va.reserve(Tword27::width);
    std::vector<Trit> vb; vb.reserve(Tword27::width);
    for (int i=0;i<Tword27::width;++i){ va.push_back(a.t[i]); vb.push_back(b.t[i]); }
    DivRR27 out{}; if (vec_zero(vb)) { out.divide_by_zero=true; return out; }
    auto rr = divmod_nearest_even(va,vb); return pack_rr27(rr.q, rr.r);
}

} // namespace t27::num
