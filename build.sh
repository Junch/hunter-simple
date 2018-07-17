#cmake -H. -B_builds -DHUNTER_STATUS_DEBUG=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build _builds --config Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1