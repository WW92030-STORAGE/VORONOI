#!/bin/bash

if [ ! -d "VENV" ]; then
    python3 -m venv VENV
fi
source VENV/bin/activate