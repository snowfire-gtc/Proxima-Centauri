#!/bin/bash

if dpkg -s qt6-base-dev >/dev/null 2>&1; then
    echo "Qt 6 development packages are installed."
else
    echo "Qt 6 development packages are not installed."
fi
