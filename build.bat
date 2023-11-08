@echo OFF
cls

cmake -S . -B build
cmake --build build --config=Release