if ./build_release.sh;
then
./bin/Release/test_nitro;
else
echo "Build failed!!!";
fi
