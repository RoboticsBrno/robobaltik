from distutils.core import setup
import py2exe
import sys

if len(sys.argv) == 1:
    sys.argv.append('py2exe')

setup(
    options = {'py2exe': {'bundle_files': 1,
                          'compressed': True,
                          'optimize': 1}},
    windows = [{'script': 'serial_comunicator.py'}],
    zipfile = None,
)
