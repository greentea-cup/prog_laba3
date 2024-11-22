#!/bin/bash
rm -f c_tree c_tokens c_errors
./release.sh $1 c_tree c_tokens c_errors && vim c_tree c_tokens c_errors -O
