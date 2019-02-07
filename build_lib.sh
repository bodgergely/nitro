mkdir -p lib

CC_PARAMS='-O2 -std=c++17 -Wall -Wextra -g'
INCLUDE='./nitro/include'

# build sharedlib libnitro.so 
g++ $CC_PARAMS -fPIC -rdynamic -shared nitro/nitro.cpp nitro/protocol.cpp -o ./lib/libnitro.so -I$INCLUDE

