#!/bin/bash
find . -name "*\.h" -o -name "*\.cpp"|xargs astyle --style=stroustrup

