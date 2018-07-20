import sys

export_symbols=sys.argv[1]
version_file=sys.argv[2]

with open(export_symbols, 'r') as syms, \
     open(version_file, 'w') as version:
     version.write("V0_1_0 {\n    global:\n")
     for sym in syms:
        version.write("        {};\n".format(sym.strip()))
     version.write("    local:\n        *;\n};\n")
