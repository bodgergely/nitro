
CC_PARAMS='-O2 -std=c++17 -Wall -Wextra -g'
INCLUDE='./nitro/include'

# build testdriver
g++ $CC_PARAMS tests/testNitro.cpp -o ./bin/testNitro -I$INCLUDE -L./lib/ -lnitro -lgtest -lgtest_main -lpthread -Wl,-rpath=/lib
