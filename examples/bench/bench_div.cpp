#include "t27/num/div.hpp"
#include "t27/num/convert.hpp"
#include "t27/num/div_long.hpp"
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <cstdlib>

using namespace t27::num;

static int getenv_i(const char* k, int defv){
    const char* v = std::getenv(k);
    if(!v || !*v) return defv;
    try { return std::stoi(v); } catch(...) { return defv; }
}

static DivMod divmod_fast_bt(std::span<const Trit> a, std::span<const Trit> b){
    long long ai = from_bt(a);
    long long bi = from_bt(b);
    long long q = ai / bi;
    long long r = ai - q * bi;
    DivMod x; x.q = to_bt(q); x.r = to_bt(r); return x;
}

int main(){
    int S = getenv_i("T27_BENCH_SAMPLES", 2000);
    int I = getenv_i("T27_BENCH_ITERS", 10);

    std::mt19937_64 rng(0xB00B1EULL);
    std::uniform_int_distribution<long long> dist(-1000000000000LL, 1000000000000LL);
    std::uniform_int_distribution<long long> dist_nz(1LL, 1000000000LL);

    std::vector<std::vector<Trit>> As, Bs;
    As.reserve(S); Bs.reserve(S);
    for (int i=0;i<S;++i){
        long long a = dist(rng);
        long long b = dist_nz(rng); if (rng()&1) b = -b;
        As.push_back(to_bt(a));
        Bs.push_back(to_bt(b));
    }

    auto time_section = [&](const char* name, auto fn){
        auto t0 = std::chrono::high_resolution_clock::now();
        long long checksum = 0;
        for (int it=0; it<I; ++it){
            for (int i=0;i<S;++i){
                auto dm = fn(As[i], Bs[i]);
                checksum += from_bt(dm.q) ^ (from_bt(dm.r)<<1);
            }
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count();
        std::cout << name << ": " << ms << " ms  (checksum=" << checksum << ")\n";
    };

    time_section("divmod_long", [&](auto& A, auto& B){ return divmod_long(A,B); });
    time_section("divmod_fast(int64)", [&](auto& A, auto& B){ return divmod_fast_bt(A,B); });

    return 0;
}
