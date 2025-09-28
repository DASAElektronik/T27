#include "t27/num/div.hpp"
#include "t27/num/convert.hpp"
#include <cassert>
#include <iostream>
#include <random>
#include <cmath>

using namespace t27::num;

static long long isgn(long long x){ return (x>0)-(x<0); }

int main_div_round(){
    std::mt19937_64 rng(0xA11D1A55ULL); // gültiges Hex-Seed
    std::uniform_int_distribution<long long> dist(-10000000000LL, 10000000000LL);
    std::uniform_int_distribution<long long> dist_nz(1LL, 1000000LL);
    for(int i=0;i<400;++i){
        long long a = dist(rng), b = dist_nz(rng); if (rng()%2) b = -b;
        auto A = to_bt(a), B = to_bt(b);

        // Euclid
        auto eu = divmod_euclid(A,B);
        long long qe = from_bt(eu.q), re = from_bt(eu.r);
        assert(qe * b + re == a);
        assert(re >= 0 && re < std::llabs(b));

        // Floor
        auto fl = divmod_floor(A,B);
        long long qf = from_bt(fl.q), rf = from_bt(fl.r);
        assert(qf * b + rf == a);
        long long qf_ref = (long long)std::floor((long double)a / (long double)b);
        assert(qf == qf_ref);

        // Ceil
        auto ce = divmod_ceil(A,B);
        long long qc = from_bt(ce.q);
        long long qc_ref = (long long)std::ceil((long double)a / (long double)b);
        assert(qc == qc_ref);

        // Trunc
        auto tr = divmod_trunc(A,B);
        long long qt = from_bt(tr.q);
        long long qt_ref = a / b; // C++ trunc toward zero
        assert(qt == qt_ref);

        // Nearest away
        auto na = divmod_nearest_away(A,B);
        long long qna = from_bt(na.q), rna = from_bt(na.r);
        long double exact = (long double)a / (long double)b;
        long long base = (long long)std::floor(exact);
        long double frac = exact - base;
        long long na_ref;
        if (frac > 0.5L) na_ref = base + 1;
        else if (frac < 0.5L) na_ref = base;
        else na_ref = base + (isgn(a)*isgn(b) >= 0 ? 1 : -1); // tie away from zero
        assert(qna == na_ref);
        assert(qna * b + rna == a);

        // Nearest even
        auto ne = divmod_nearest_even(A,B);
        long long qne = from_bt(ne.q), rne = from_bt(ne.r);
        long long ne_ref;
        if (frac > 0.5L) ne_ref = base + 1;
        else if (frac < 0.5L) ne_ref = base;
        else {
            // tie to even
            ne_ref = (base % 2 == 0) ? base : base + (isgn(a)*isgn(b) >= 0 ? 1 : -1);
        }
        assert(qne == ne_ref);
        assert(qne * b + rne == a);
    }
    std::cout << "div rounding OK\n";
    return 0;
}
