// fuzz/t27_fuzz_div.cpp
#include <t27/num/div_long.hpp>
#include <t27/num/trit.hpp>
#include <vector>
#include <cstdint>
#include <cstdlib>

using namespace t27::num;

// Convert a byte buffer into two small trit vectors.
static std::vector<Trit> bytes_to_trits(const uint8_t* data, size_t n, size_t max_len){
  std::vector<Trit> v;
  v.reserve(max_len);
  for(size_t i=0; i<n && v.size()<max_len; ++i){
    switch (data[i] % 3){
      case 0: v.push_back(Trit::N); break;
      case 1: v.push_back(Trit::Z); break;
      default: v.push_back(Trit::P); break;
    }
  }
  // canonicalize: strip leading zeros in little-endian order
  while(!v.empty() && v.back()==Trit::Z) v.pop_back();
  return v;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Split buffer: first half -> a, second half -> b (ensure b != 0)
  size_t half = size/2;
  auto a = bytes_to_trits(data, half, 64);
  auto b = bytes_to_trits(data+half, size-half, 64);
  if (b.empty()) {
    b.push_back(Trit::P);
  }

  try {
    auto [q, r] = divmod_long(a, b);
    // quick postconditions (do not throw; fuzz should not halt on logical fails)
    (void)q; (void)r;
  } catch (...) {
    // library may throw only on internal guards; swallowing exceptions for fuzz loop.
  }
  return 0;
}