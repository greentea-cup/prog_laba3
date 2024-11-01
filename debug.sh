#!/bin/bash
cmake --build build/Debug --preset Debug && gdb ./build/Debug/app
