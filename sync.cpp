#include "sync.h"
#include <mutex>
#include <shared_mutex>
#include <fstream>
#include <chrono>
#include <random>
#include <stdexcept>
#include <cassert>
#include <iomanip>
#include <syncstream>

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

static void write_ops(std::ofstream& out, op_kind k, int fld, int val) {
    switch (k) {
        case op_kind::read:out << "read "   << fld << "\n"; break;
        case op_kind::write:out << "write "  << fld << " " << val << "\n"; break;
        case op_kind::as_string:out << "string"  << "\n"; break;
    }
}

static void generate_weighted(const std::string& filename,
                              int p_r0, int p_w0, int p_r1, int p_w1, int p_str,
                              size_t n_ops)
{
    assert(p_r0 >= 0 && p_w0 >= 0 && p_r1 >= 0 && p_w1 >= 0 && p_str >= 0);
    const int total = p_r0 + p_w0 + p_r1 + p_w1 + p_str;
    assert(total == 100 && "freqs must sum to 100");

    std::ofstream out(filename);
    if (!out) throw std::runtime_error("cannot open " + filename);

    std::mt19937_64 rng(0xC0FFEE + std::hash<std::string>{}(filename));
    std::uniform_int_distribution<int> pick(1, 100);
    std::uniform_int_distribution<int> val(1, 1000);

    for (size_t i = 0; i < n_ops; ++i) {
        int r = pick(rng);
        if ((r -= p_r0) <= 0) { write_ops(out, op_kind::read, 0, 0); continue; }
        if ((r -= p_w0) <= 0) { write_ops(out, op_kind::write, 0, val(rng)); continue; }
        if ((r -= p_r1) <= 0) { write_ops(out, op_kind::read, 1, 0); continue; }
        if ((r -= p_w1) <= 0) { write_ops(out, op_kind::write, 1, val(rng)); continue; }
        write_ops(out, op_kind::as_string, -1, 0);
    }

    if (!out) { throw std::runtime_error("I/O error while writing " + filename); }
}

void generate_case_A_variant16(const std::string& base_name, int tid, size_t n_ops) {
    generate_weighted(base_name + "_A_t" + std::to_string(tid) + ".txt",
                      20, 5, 20, 5, 50, n_ops);
}

void generate_case_B_uniform(const std::string& base_name, int tid, size_t n_ops) {
    generate_weighted(base_name + "_B_t" + std::to_string(tid) + ".txt",
                      20, 20, 20, 20, 20, n_ops);
}

void generate_case_C_skewed(const std::string& base_name, int tid, size_t n_ops) {
    generate_weighted(base_name + "_C_t" + std::to_string(tid) + ".txt",
                      5, 40, 5, 40, 10, n_ops);
}

std::pair<action*, size_t> load_actions(const std::string& filename) {
    std::ifstream f(filename);
    if (!f) throw std::runtime_error("cannot open " + filename);
    size_t cnt = 0;
    std::string tmp;
    while (std::getline(f, tmp)) ++cnt;

    action* arr = new action[cnt];
    
    f.close();
    std::ifstream g(filename);
    size_t i = 0;
    while (g) {
        std::string op;
        if (!(g >> op)) break;
        if (op == "read") {
            int fld; g >> fld;
            arr[i++] = action{op_kind::read, fld, 0};
        } else if (op == "write") {
            int fld, v; g >> fld >> v;
            arr[i++] = action{op_kind::write, fld, v};
        } else if (op == "string") {
            arr[i++] = action{op_kind::as_string, -1, 0};
        } else {
            std::string skip; std::getline(g, skip);
        }
    }
    return {arr, i};
}