#!/bin/bash

cmake .
make
if [ $? == 2 ]; then
    echo "Exiting..."
else
    ./ComputerVision
fi