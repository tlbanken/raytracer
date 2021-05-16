#!/bin/bash

set -e

make

for i in {1..10}; do
    ./raytracer -ol > "out$i.ppm"
done

