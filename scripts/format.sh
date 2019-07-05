#!/bin/bash

cd "$(dirname "$0")"

echo -n "Running dos2unix     "
find .. -name "*\.h" -o -name "*\.cpp"|grep -v bundled|xargs -I {} sh -c "dos2unix '{}' 2>/dev/null; echo -n '.'"
echo
echo -n "Running clang-format "
find .. -name "*\.h" -o -name "*\.cpp"|grep -v bundled|xargs -I {} sh -c "clang-format -i {}; echo -n '.'"
echo


