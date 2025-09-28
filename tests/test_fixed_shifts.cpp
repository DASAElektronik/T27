#include "t27/num/fixed.hpp"
#include "t27/num/convert.hpp"
#include <cassert>
#include <iostream>
#include <random>

using namespace t27::num;

static Tword27 make_word(long long x){ auto v=to_bt(x); return to_word27(v); }
static bool any_nonzero_ms(const Tword27& a, int k){ for(int i=Tword27::width-k;i<Tword27::width;++i) if(i>=0 && a.t[i]!=Trit::Z) return true; return false; }
static bool any_nonzero_ls(const Tword27& a, int k){ for(int i=0;i<k && i<Tword27::width;++i) if(a.t[i]!=Trit::Z) return true; return false; }
static Tword27 ref_shl(const Tword27& a, int k){ Tword27 r{}; for(int i=Tword27::width-1;i>=0;--i) r.t[i]=(i-k>=0)?a.t[i-k]:Trit::Z; return r; }
static Tword27 ref_lshr(const Tword27& a, int k){ Tword27 r{}; for(int i=0;i<Tword27::width;++i) r.t[i]=(i+k<Tword27::width)?a.t[i+k]:Trit::Z; return r; }
static Tword27 ref_shr(const Tword27& a, int k){ Tword27 r{}; Trit ms=a.t[Tword27::width-1]; for(int i=0;i<Tword27::width;++i) r.t[i]=(i+k<Tword27::width)?a.t[i+k]:ms; return r; }

int main_fixed_shifts(){
    std::mt19937_64 rng(0x12345678ULL);
    std::uniform_int_distribution<long long> dist(-1000000000000LL, 1000000000000LL);
    for(int i=0;i<100;++i){
        long long x=dist(rng); Tword27 a=make_word(x);
        for(int k : {0,1,2,3,5,10,26,27,28}){
            auto [sl, fl]=shl27(a,k);
            bool ovf_shl = (k>0) && ((k>=Tword27::width) ? ([&](){for(int j=0;j<Tword27::width;++j) if(a.t[j]!=Trit::Z) return true; return false;}()) : any_nonzero_ms(a,k));
            assert(sl.to_string()==ref_shl(a,k).to_string() && fl.overflow==ovf_shl);
            auto [lr, flr]=lshr27(a,k);
            bool ovf_lshr = (k>0) && ((k>=Tword27::width) ? ([&](){for(int j=0;j<Tword27::width;++j) if(a.t[j]!=Trit::Z) return true; return false;}()) : any_nonzero_ls(a,k));
            assert(lr.to_string()==ref_lshr(a,k).to_string() && flr.overflow==ovf_lshr);
            auto [sr, fsr]=shr27(a,k);
            bool ovf_shr = ovf_lshr;
            assert(sr.to_string()==ref_shr(a,k).to_string() && fsr.overflow==ovf_shr);
        }
    }
    std::cout << "Fixed-width shift tests passed.\n";
    return 0;
}