@echo off
echo ====================================================================
echo   Building ERL-HUB (Post-Earthquake Emergency Relief Logistics Hub)
echo   Compiler: g++ (C++17)
echo ====================================================================
g++ -std=c++17 -Wall -Wextra -Iinclude src/Cargo.cpp src/Vehicle.cpp src/Depot.cpp src/main.cpp -o erl_hub.exe
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] erl_hub.exe compiled successfully!
    echo Run .\erl_hub.exe to launch the application.
) else (
    echo [ERROR] Compilation failed.
)
