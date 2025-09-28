#pragma once
/**
 * \file
 * \ingroup numerics
 * \brief Converters between 64-bit integers and balanced-trit vectors.
 *
 * Balanced trits in T-27 use the enum \c Trit with values {N=-1, Z=0, P=+1}.
 * Vectors are little-endian: index i stores coefficient of 3^i.
 *
 * These functions are header-only and have no runtime dependencies
 * beyond the public T-27 headers.
 */
#include <t27/num/trit.hpp>
#include <vector>
#include <string>
#include <stdexcept>
#include <span>
#include <cstdint>

namespace t27 { namespace util {

using t27::num::Trit;

/** Convert a little-endian trit vector to int64 (with range checking). */
inline long long to_int64(std::span<const Trit> v){
    __int128 acc = 0;
    __int128 pow = 1;
    for (Trit t : v){
        int d = (t==Trit::P? +1 : (t==Trit::N? -1 : 0));
        acc += (__int128)d * pow;
        pow *= 3;
    }
    // clamp / check
    const __int128 min64 = static_cast<__int128>(std::numeric_limits<long long>::min());
    const __int128 max64 = static_cast<__int128>(std::numeric_limits<long long>::max());
    if (acc < min64 || acc > max64) throw std::overflow_error("to_int64: value out of int64 range");
    return static_cast<long long>(acc);
}

/** Convert a signed 64-bit integer to balanced-trit vector (little-endian). */
inline std::vector<Trit> from_int64(long long x){
    std::vector<Trit> out;
    if (x == 0) return out; // canonical zero = empty vector
    // Balanced ternary conversion with carry adjustment.
    while (x != 0){
        long long r = x % 3;         // can be negative in C++
        x /= 3;
        if (r < 0){ r += 3; x -= 1; } // make remainder non-negative
        if (r == 0){
            out.push_back(Trit::Z);
        } else if (r == 1){
            out.push_back(Trit::P);
        } else { // r == 2  <=>  digit -1 with carry +1
            out.push_back(Trit::N);
            x += 1;
        }
    }
    // canonicalize: strip trailing zeros in little-endian (should not occur here)
    while (!out.empty() && out.back() == Trit::Z) out.pop_back();
    return out;
}

/** Parse a compact string of '+', '0', '-' into trit vector (little-endian).
 *  The first character is the least significant trit (3^0).
 *  Example: "+-0" -> [P, N, Z].
 */
inline std::vector<Trit> parse_bt(std::string_view s){
    std::vector<Trit> v; v.reserve(s.size());
    for(char ch : s){
        switch(ch){
            case '+': v.push_back(Trit::P); break;
            case '0': v.push_back(Trit::Z); break;
            case '-': v.push_back(Trit::N); break;
            default: throw std::invalid_argument("parse_bt: expected '+','0','-'");
        }
    }
    // canonicalize
    while (!v.empty() && v.back()==Trit::Z) v.pop_back();
    return v;
}

/** Serialize a trit vector to compact '+0-' string (little-endian order). */
inline std::string to_string_bt(std::span<const Trit> v){
    std::string s; s.reserve(v.size());
    for(Trit t : v){
        s.push_back(t==Trit::P? '+' : (t==Trit::N? '-' : '0'));
    }
    return s;
}

}} // namespace t27::util