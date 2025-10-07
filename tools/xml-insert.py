#!/usr/bin/env python3
#
# Utility script to insert an xml snippet into an existing document
# 
# Re-implements xml_insert.sh in python with extra options to send the 
# output to a new file.
# 
# Copyright John Marshall 2020
#

from __future__ import print_function

import os
import sys
import argparse

class Args():
    def __init__(self):
        parser = argparse.ArgumentParser()
        parser.add_argument(
            '--output',
            metavar='OUTPUT_FILE',
            help='output file - otherwise output to STDOUT'
        )
        parser.add_argument(
            '--insert',
            action='append',
            nargs=2,
            required=True,
            metavar=('TAG', 'INSERT_FILE'),
            help='insert file tag and file name'
        )
        parser.add_argument(
            'FILE',
            metavar='INPUT_FILE',
            help='input file'
        )

        self.input = os.path.realpath(parser.parse_args().FILE)
        if parser.parse_args().output:
            self.output = os.path.realpath(parser.parse_args().output)
        else:
            self.output = None
        self.inserts = parser.parse_args().insert


def main():
    args = Args()

    try:
        in_file = open(args.input, mode='r', encoding='utf-8')
    except IOError:
        print('cannot access input file ' + args.input, 
              file=sys.stderr)
        sys.exit(1)

    doc = in_file.read().splitlines()
    in_file.close()

    # get insert files
    inserts = args.inserts
    for insert in inserts:
        ins_tag = '<!--' + insert[0] + '-->'

        # find tag instances in input file
        indices = []
        for i, line in enumerate(doc):
            if ins_tag in line:
                indices.append(i)

        if not indices:
            print(ins_tag + ' not in input file - skipping',
                  file=sys.stderr)
            continue

        # read in insert file
        try:
            ins_file = open(os.path.realpath(insert[1]), mode='r', encoding='utf-8')
        except IOError:
            print ('cannot open insert file %s - skipping' % insert[1],
                  file=sys.stderr)
            continue

        if ins_file:
            ins_doc = ins_file.read().splitlines()
            ins_file.close()

        # insert in reverse order so that remaining inert positions 
        # remain valid
        for index in reversed(indices):
            doc[index+1:index+1] = ins_doc

    # output to file or stdout
    if args.output:
        try:
            out_file = open(os.path.realpath(args.output), mode='w', encoding='utf-8')
        except IOError:
            print('cannot open output file %s' % args.output)
            sys.exit(1)
    else:
        out_file = sys.stdout

    for line in doc:
        print(line, file=out_file)

    sys.exit(0)


if __name__ == "__main__":
  main()