#include "t27/num/div3k.hpp"
#include "t27/num/convert.hpp"
#include <cassert>
#include <iostream>
#include <random>

using namespace t27::num;

static long long pow3(int k){ long long p=1; for(int i=0;i<k;++i) p*=3; return p; }

int main_div3k(){
    std::mt19937_64 rng(0xBADC0DEULL);
    std::uniform_int_distribution<long long> dist(-1000000000000LL, 1000000000000LL);

    for(int i=0;i<120;++i){
        long long x = dist(rng);
        auto bx = to_bt(x);

        for (int k : {0,1,2,3,4,5,10,15}){
            auto dm = divmod3k_balanced(bx, k);
            long long d = pow3(k);
            long long q_round = from_bt(dm.q);
            long long r_bal  = from_bt(dm.r);
            assert(q_round * d + r_bal == x);
            long long R = (d - 1) / 2;
            assert(r_bal >= -R && r_bal <= +R);
        }
    }
    std::cout << "Div3k tests passed.\n";
    return 0;
}