#include "t27/num/fixed.hpp"
#include "t27/num/convert.hpp"
#include <cassert>
#include <iostream>

using namespace t27::num;
static long long pow3(int k){ long long p=1; for(int i=0;i<k;++i) p*=3; return p; }
static Tword27 make_word(long long x){ auto v=to_bt(x); return to_word27(v); }
static long long word_val(const Tword27& w){ return from_bt(std::span<const Trit>(w.t.data(), w.t.size())); }

int main_fixed(){
    const long long MOD=pow3(27); const long long MAX=(MOD-1)/2; const long long MIN=-MAX;
    { auto [r,f]=add27(make_word(MAX), make_word(1)); assert(f.overflow && word_val(r)==MIN); }
    { auto [r,f]=add27(make_word(MIN), make_word(-1)); assert(f.overflow && word_val(r)==MAX); }
    { auto [r,f]=mul27(make_word(pow3(14)), make_word(pow3(14))); assert(f.overflow && word_val(r)==3); }
    std::cout << "Fixed-width tests passed.\n";
    return 0;
}