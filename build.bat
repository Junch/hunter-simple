cmake -H. -B_builds -DHUNTER_STATUS_DEBUG=ON -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=OFF -DBUILD_SAMPLES=OFF
cmake --build _builds --config Debug --parallel 4 -- /verbosity:detailed