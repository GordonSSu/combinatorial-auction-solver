#!/bin/sh

GUROBI_HOME=/Library/gurobi951/macos_universal2

export PYTHONPATH=$GUROBI_HOME/lib
export PYTHONSTARTUP=/Library/gurobi951/macos_universal2/lib/gurobi.py

$GUROBI_HOME/bin/python3.9 "$@"
