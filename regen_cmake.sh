#!/bin/bash
rm -r build
cmake --preset Release -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --preset Debug -DCMAKE_BUILD_TYPE=Debug
[ -f compile_commands.json ] && rm compile_commands.json
ln -s build/Release/compile_commands.json .
