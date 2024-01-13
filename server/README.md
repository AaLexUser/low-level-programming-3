# LLP Database
## Quick Start
To run tests:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_BENCHMARK=OFF -DBUILD_TESTING=ON ..
cmake --build .  
ctest
``` 