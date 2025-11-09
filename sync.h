#pragma once
#include <shared_mutex>
#include <string>
#include <sstream>
#include <utility>

enum class op_kind { read, write, as_string };

struct action {
    op_kind kind{};
    int field{ -1 };   
    int value{ 0 };   
};

class two_fields {
    public:
    two_fields() = default;
    explicit two_fields(int f0, int f1) : f0_(f0), f1_(f1) {}

    two_fields(const two_fields&) = delete;
    two_fields& operator=(const two_fields&) = delete;

    int  get0() const;
    void set0(int v);

    int  get1() const;
    void set1(int v);

    explicit operator std::string() const;
    
    private:
    int f0_{0};
    int f1_{0};

    mutable std::shared_mutex m0_;
    mutable std::shared_mutex m1_;
};

long long run_actions(const action* begin, const action* end, two_fields& obj);

void generate_case_A_variant16(const std::string& base_name, int tid, size_t n_ops);
void generate_case_B_uniform(const std::string& base_name, int tid, size_t n_ops);
void generate_case_C_skewed(const std::string& base_name, int tid, size_t n_ops);

std::pair<action*, size_t> load_actions(const std::string& filename);