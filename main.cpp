#include "sync.h"
#include <thread>
#include <vector>
#include <iostream>
#include <syncstream>
#include <cstdio>
#include <algorithm>   
#include <exception> 

static void run_suite(const std::string& label,
                      const std::string& base,
                      size_t n_ops_per_thread,
                      int n_threads,
                      int repeats)
{
    for (int t = 0; t < n_threads; ++t) {
        if (label == "A") generate_case_A_variant16(base, t, n_ops_per_thread);
        else if (label == "B") generate_case_B_uniform(base, t, n_ops_per_thread);
        else generate_case_C_skewed(base, t, n_ops_per_thread);
    }

    std::vector<std::pair<action*, size_t>> data;
    data.reserve(n_threads);
    for (int t = 0; t < n_threads; ++t) {
        const std::string fn = base + "_" + label + "_t" + std::to_string(t) + ".txt";
        try {
            data.push_back(load_actions(fn));
        } catch (const std::bad_alloc&) {
            std::cerr << "out of memory while loading " << fn << "\n";
            return; 
        } catch (const std::exception& e) {
            std::cerr << "error while loading " << fn << ": " << e.what() << "\n";
            return;
        }
    }

    two_fields obj(0, 0);

    std::vector<long long> sum_elapsed(n_threads, 0);

    auto run_once = [&](){
        std::vector<long long> elapsed(n_threads, 0);

        auto worker = [&](int tid){
            const auto [ptr, cnt] = data[tid];
            elapsed[tid] = run_actions(ptr, ptr + cnt, obj);
        };

        if (n_threads == 1) {
            worker(0);
        } else if (n_threads == 2) {
            std::jthread th0(worker, 0);
            std::jthread th1(worker, 1);
        } else {
            std::jthread th0(worker, 0);
            std::jthread th1(worker, 1);
            std::jthread th2(worker, 2);
        }

        for (int t = 0; t < n_threads; ++t) sum_elapsed[t] += elapsed[t];
    };

    for (int r = 0; r < repeats; ++r) run_once();

    long long total_sum = 0;
    for (int t = 0; t < n_threads; ++t) total_sum += sum_elapsed[t];

    std::osyncstream(std::cout)
        << "[case " << label << "] threads=" << n_threads
        << "  per-thread ops=" << n_ops_per_thread
        << "  repeats=" << repeats << "\n";

    for (int t = 0; t < n_threads; ++t) {
        const std::string fn = base + "_" + label + "_t" + std::to_string(t) + ".txt";
        std::osyncstream(std::cout)
            << "  file=" << fn
            << "  elapsed_avg_ns=" << (sum_elapsed[t] / (repeats ? repeats : 1)) << "\n";
    }

    std::osyncstream(std::cout)
        << "  total_elapsed_avg_ns=" << (total_sum / (repeats ? repeats : 1)) << "\n";

    for (auto& [ptr, cnt] : data) delete[] ptr;
}

int main(int argc, char** argv) {
    size_t n_ops = 500000;
    int repeats = 1;
    try {
        if (argc > 1) n_ops = static_cast<size_t>(std::stoull(argv[1]));
        if (argc > 2) repeats = std::max(1, std::stoi(argv[2]));
    } catch (const std::exception& e) {
        std::cerr << "invalid arguments: " << e.what() << "\n";
        return 1;
    }

    for (int thr : {1,2,3}) {
        run_suite("A", "ops", n_ops, thr, repeats); 
        run_suite("B", "ops", n_ops, thr, repeats); 
        run_suite("C", "ops", n_ops, thr, repeats);
    }

    two_fields dbg(1, 2);
    std::cout << "sample string(): " << static_cast<std::string>(dbg) << "\n";
}