:: Usage compile.bat <experiment>
:: default Windows compiler: clang
clang -o app.exe -g -Wall -Wextra -std=c++17 -lUser32 -lGdi32 src\win-backend.cpp %*
