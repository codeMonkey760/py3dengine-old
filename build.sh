if [ -d "cmake-build-debug" ]; then
	rm -rf cmake-build-debug
fi

conan install -of cmake-build-debug --build missing .
cd cmake-build-debug
cmake -DCMAKE_BUILD_TYPE=Debug -DPY3D_TEST_PROJECT_LOCATION="/home/sp/repos/py3dengine-test" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -S ../ -B .
make
