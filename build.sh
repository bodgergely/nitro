rm -rf ./lib
rm -rf ./bin

mkdir -p ./lib
mkdir -p ./bin

./build_lib.sh && ./build_tests.sh && ./build_app.sh
