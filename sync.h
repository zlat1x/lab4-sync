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

class two_fields;

long long run_actions(const action* begin, const action* end, two_fields& obj);

void generate_case_A_variant16(const std::string& base_name, int tid, size_t n_ops);
void generate_case_B_uniform   (const std::string& base_name, int tid, size_t n_ops);
void generate_case_C_skewed    (const std::string& base_name, int tid, size_t n_ops);

std::pair<action*, size_t> load_actions(const std::string& filename);