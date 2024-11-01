#!/bin/bash
cmake --build build/Release --preset Release && ./build/Release/app $@
