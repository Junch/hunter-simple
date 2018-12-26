cmake -H. -B_builds -DHUNTER_STATUS_DEBUG=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_TESTS=OFF -DBUILD_SAMPLES=OFF
cmake --build _builds --config Debug --parallel 8 -- VERBOSE=1 2>&1 | tee build.log
