# Lab 4 — Sync (variant 16)
Build:
  g++ -std=c++23 -O2 -Wall -Wextra -pthread main.cpp sync.cpp -o lab4_sync.exe

Run (усереднення):
  lab4_sync.exe 500000 3

Output:
  друкуються блоки [case A/B/C] для threads=1..3 зрядком total_elapsed_avg_ns