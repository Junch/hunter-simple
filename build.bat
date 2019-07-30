cmake -H. -B_builds -DHUNTER_STATUS_DEBUG=ON -DBUILD_TESTS=OFF -DBUILD_SAMPLES=OFF
cmake --build _builds --config Debug --parallel 8 -- /verbosity:detailed