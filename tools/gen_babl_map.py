#!/usr/bin/env python3

import sys

export_symbols=sys.argv[1]
version_file=sys.argv[2]
version_file_clang=sys.argv[2] + ".clang"

with open(export_symbols, 'r') as syms, \
     open(version_file, 'w') as version:
     version.write("V0_1_0 {\n    global:\n")
     next(syms)
     for sym in syms:
       final_sym = sym.replace('  ', '')
       version.write("        {};\n".format(final_sym.strip()))
     version.write("    local:\n        *;\n};\n")

with open(export_symbols, 'r') as syms, \
     open(version_file_clang, 'w') as version:
     next(syms)
     for sym in syms:
       final_sym = sym.replace('  ', '')
       version.write("_{}\n".format(final_sym.strip()))
