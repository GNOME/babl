#!/usr/bin/env python3
import os
import shutil
import sys

builddir = sys.argv[1]
distdir = os.environ['MESON_DIST_ROOT']
git_version_h_filename = 'git-version.h'

shutil.copyfile(os.path.join(builddir, git_version_h_filename),
                os.path.join(distdir, 'babl', git_version_h_filename))
