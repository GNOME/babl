#!/usr/bin/env python3
import os
import sys

sys.exit(not os.path.isdir(os.path.join(os.path.dirname(os.path.realpath(__file__)), '.git')))
