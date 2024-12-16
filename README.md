## Py3DEngine
3D graphics engine scriptable with python

### Compiling From Source
Please keep in mind that these are rough notes so that I can remember the commands
1) Install dependencies with `conan install -of cmake-build-debug --build missing .`
2) Navigate to build folder, in this case `cmake-build-debug`
3) Run cmake with the following command `cmake -DCMAKE_BUILD_TYPE=Debug -DPY3D_TEST_PROJECT_LOCATION="/home/sp/repos/py3dengine-test" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -S ../ -B .`
4) Kick off the build with `make`