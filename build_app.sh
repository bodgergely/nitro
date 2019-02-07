
CC_PARAMS='-O2 -std=c++17 -Wall -Wextra -g'
INCLUDE='./nitro/include'

# build nitro app
mkdir -p bin
g++ $CC_PARAMS app/main.cpp -o ./bin/nitro -I$INCLUDE -L./lib/ -lnitro -Wl,-rpath=lib/

