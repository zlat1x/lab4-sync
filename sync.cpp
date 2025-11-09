#include "sync.h"
#include <mutex>
#include <shared_mutex>
#include <fstream>
#include <chrono>

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

long long run_actions(const action* begin, const action* end, two_fields& obj) {
    using clock = std::chrono::high_resolution_clock;
    const auto t0 = clock::now();

    for (auto it = begin; it != end; ++it) {
        switch (it->kind) {
            case op_kind::read:
                if (it->field == 0) { (void)obj.get0(); }
                else { (void)obj.get1(); }
                break;
            case op_kind::write:
                if (it->field == 0) { obj.set0(it->value); }
                else { obj.set1(it->value); }
                break;
            case op_kind::as_string: {
                std::string s = static_cast<std::string>(obj);
                (void)s; 
            } break;
        }
    }

    const auto t1 = clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
}