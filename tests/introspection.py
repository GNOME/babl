#!/usr/bin/env python

import unittest

class TestSanity(unittest.TestCase):

    def test_import(self):
        import gi
        from gi.repository import Babl

    def test_init(self):
        import gi
        from gi.repository import Babl
        Babl.init()
        Babl.exit()

if __name__ == '__main__':
    unittest.main()
