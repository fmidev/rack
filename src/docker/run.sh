#!/bin/bash

# Default start script
#
# Markus.Peura@fmi.fi

df

ls -ltr

pwd -P

whoami

echo PATH=$PATH

echo "parameters: $*"

rack --help


