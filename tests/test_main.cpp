#include <iostream>
#include <exception>

int main_divmod();
int main_div3k();
int main_fixed();
int main_fixed_shifts();
int main_div_round();

static int run_test(const char* name, int(*fn)()){
    std::cout << "[ RUN      ] " << name << std::endl;
    int rc = 0;
    try { rc = fn(); }
    catch(const std::exception& e){
        std::cout << "[  ERROR   ] " << name << " threw std::exception: " << e.what() << std::endl;
        rc = 100;
    }
    catch(...){
        std::cout << "[  ERROR   ] " << name << " threw unknown exception" << std::endl;
        rc = 101;
    }
    if (rc == 0) std::cout << "[       OK ] " << name << " (rc=0)" << std::endl;
    else         std::cout << "[  FAILED  ] " << name << " (rc=" << rc << ")" << std::endl;
    return rc;
}

int main(){
    std::cout << "=== T27 test runner (verbose) ===" << std::endl;
    int rc = 0;
    rc |= run_test("divmod",          &main_divmod);
    rc |= run_test("div3k",           &main_div3k);
    rc |= run_test("fixed",           &main_fixed);
    rc |= run_test("fixed_shifts",    &main_fixed_shifts);
    rc |= run_test("div_round",       &main_div_round);
    std::cout << "=== Summary: rc=" << rc << " ===" << std::endl;
    return rc;
}
