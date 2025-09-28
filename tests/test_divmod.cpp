#include "t27/num/div.hpp"
#include "t27/num/convert.hpp"
#include "t27/num/div_long.hpp"
#include <cassert>
#include <iostream>
#include <random>
#include <cstdlib>

using namespace t27::num;

static int getenv_i(const char* k, int defv){
    const char* v = std::getenv(k);
    if(!v || !*v) return defv;
    try { return std::stoi(v); } catch(...) { return defv; }
}

static void print_bt(const std::vector<Trit>& v){
    for (int i=(int)v.size()-1;i>=0;--i){
        char c = (v[i]==Trit::N?'-':(v[i]==Trit::Z?'0':'1'));
        std::cout << c;
    }
}

int main_divmod(){
    const bool verbose = getenv_i("T27_VERBOSE", 0) != 0;
    int N = getenv_i("T27_PROP_N", 1200);

    std::mt19937_64 rng(0xA11D00DULL);
    std::uniform_int_distribution<long long> dist(-1000000000000LL, 1000000000000LL);
    std::uniform_int_distribution<long long> dist_nz(1LL, 1000000000LL);

    if (verbose) std::cout << "[divmod] seed=0xA11D00DULL, N="<<N<<"\n";

    // Deterministic probes
    long long probes[][2] = {
        { 0,  1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1},
        { 7,  3}, {7, -3}, {-7, 3}, {-7, -3},
        { 9,  3}, {9, -3}, {-9, 3}, {-9, -3},
        { 123456789,  12345}, {123456789, -12345},
        {-123456789,  12345}, {-123456789, -12345}
    };
    for (auto& pr : probes){
        long long a = pr[0], b = pr[1];
        auto A = to_bt(a), B = to_bt(b);
        auto f = divmod(A,B);
        auto l = divmod_long(A,B);
        long long qf = from_bt(f.q), rf = from_bt(f.r);
        long long ql = from_bt(l.q), rl = from_bt(l.r);
        if (verbose){
            std::cout << "probe: a="<<a<<" b="<<b<<" | ";
            std::cout << "fast: "<<a<<" = "<<qf<<"*"<<b<<" + "<<rf<<" | ";
            std::cout << "long: "<<a<<" = "<<ql<<"*"<<b<<" + "<<rl<<"\n";
        }
        if (!(qf*b+rf==a && ql*b+rl==a && qf==ql && rf==rl)){
            std::cout << "[COUNTEREXAMPLE probe] a="<<a<<" b="<<b<<"\n";
            return 10;
        }
    }

    for (int i=0;i<N;++i){
        long long a = dist(rng);
        long long b = dist_nz(rng); if (rng()&1) b = -b;

        auto A = to_bt(a);
        auto B = to_bt(b);

        auto dm_fast = divmod(A, B);
        auto dm_long = divmod_long(A, B);

        long long qf = from_bt(dm_fast.q);
        long long rf = from_bt(dm_fast.r);
        long long ql = from_bt(dm_long.q);
        long long rl = from_bt(dm_long.r);

        if (verbose && i < 10){
            std::cout << "case["<<i<<"]: "<<a<<" / "<<b<<" -> ";
            std::cout << "fast: "<<a<<"="<<qf<<"*"<<b<<"+"<<rf<<" | ";
            std::cout << "long: "<<a<<"="<<ql<<"*"<<b<<"+"<<rl<<"\n";
        }

        if (!(qf * b + rf == a && ql * b + rl == a && qf == ql && rf == rl)){
            std::cout << "[COUNTEREXAMPLE rnd i="<<i<<"] a=" << a << "  b=" << b << "\n";
            std::cout << "fast: q=" << qf << " r=" << rf << "  ternary(A)=";
            print_bt(A); std::cout << "  ternary(B)="; print_bt(B); std::cout << "\n";
            std::cout << "long: q=" << ql << " r=" << rl << "\n";
            return 11;
        }
    }

    std::cout << "General divmod tests passed.\n";
    return 0;
}
