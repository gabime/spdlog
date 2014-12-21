#!/bin/bash
find . -name "*\.h" -o -name "*\.cpp"|xargs dos2unix
find . -name "*\.h" -o -name "*\.cpp"|xargs astyle -n -c -t4 -A1 


