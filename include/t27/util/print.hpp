#pragma once
/**
 * \file
 * \ingroup numerics
 * \brief Small pretty-printers for balanced-trit vectors.
 */
#include <t27/num/trit.hpp>
#include <t27/num/fixed.hpp>
#include <vector>
#include <span>
#include <string>
#include <sstream>

namespace t27 { namespace util {

using t27::num::Trit;
using t27::num::Tword27;

/** Compact '+0-' string (LSB-first). */
inline std::string str_compact(std::span<const Trit> v){
    std::string s; s.reserve(v.size());
    for (auto t : v) s.push_back(t==Trit::P? '+' : (t==Trit::N? '-' : '0'));
    if (s.empty()) s = "0"; // show zero
    return s;
}

/** Pretty string with powers: "+ 3^0, - 3^2, ..." for debugging. */
inline std::string str_expanded(std::span<const Trit> v){
    std::ostringstream oss;
    bool first = true;
    for(size_t i=0;i<v.size();++i){
        int d = (v[i]==Trit::P? +1 : (v[i]==Trit::N? -1 : 0));
        if (!d) continue;
        if (!first) oss << ", ";
        oss << (d>0? "+ " : "- ") << "3^" << i;
        first = false;
    }
    if (first) return "0";
    return oss.str();
}

}} // namespace t27::util