#include "sync.h"
#include <mutex>
#include <shared_mutex>
#include <fstream>

int two_fields::get0() const {
    std::shared_lock<std::shared_mutex> lk(m0_);
    return f0_;
}

void two_fields::set0(int v) {
    std::unique_lock<std::shared_mutex> lk(m0_); 
    f0_ = v;
}

int two_fields::get1() const {
    std::shared_lock<std::shared_mutex> lk(m1_);
    return f1_;
}

void two_fields::set1(int v) {
    std::unique_lock<std::shared_mutex> lk(m1_); 
    f1_ = v;
}

two_fields::operator std::string() const {
    std::shared_lock<std::shared_mutex> l0(m0_, std::defer_lock);
    std::shared_lock<std::shared_mutex> l1(m1_, std::defer_lock);
    std::lock(l0, l1); 

    std::ostringstream oss;
    oss << "f0=" << f0_ << ", f1=" << f1_;
    return oss.str();
}