#!/bin/bash
make clean
make
EXAMPLE_ENV="well shit"
python3 py/router.py