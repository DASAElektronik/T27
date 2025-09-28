#include "t27/num/div.hpp"
#include "t27/num/div_long.hpp"     // <- unsere echte Long-Division
#include <vector>

namespace t27::num {

    DivMod divmod(std::span<const Trit> a, std::span<const Trit> b) {
        // delegiere komplett an die header-only Long-Division (mit Safety-Fallback)
        return divmod_long(a, b);
    }

    DivMod27 divmod27(const Tword27& a, const Tword27& b) {
        std::vector<Trit> va; va.reserve(Tword27::width);
        std::vector<Trit> vb; vb.reserve(Tword27::width);
        for (int i = 0; i < Tword27::width; ++i) { va.push_back(a.t[i]); vb.push_back(b.t[i]); }

        DivMod27 out{};
        bool bzero = true; for (auto t : vb) if (t != Trit::Z) { bzero = false; break; }
        if (bzero) { out.divide_by_zero = true; return out; }

        auto res = divmod_long(va, vb);

        bool rz = true; for (auto t : res.r) if (t != Trit::Z) { rz = false; break; }
        out.inexact = !rz;
        out.overflow_q = (int)res.q.size() > Tword27::width;

        for (int i = 0; i < Tword27::width && i < (int)res.q.size(); ++i) out.q.t[i] = res.q[i];
        for (int i = 0; i < Tword27::width && i < (int)res.r.size(); ++i) out.r.t[i] = res.r[i];
        return out;
    }

} // namespace t27::num
