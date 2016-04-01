from distutils.core import setup
import py2exe
import sys

if len(sys.argv) == 1:
    sys.argv.append('py2exe')
else:
    if sys.argv[1] != 'py2exe':
        print 'Are you sure you realy want {0}?\nThis setup is intended for creating executable by py2exe. I realy do not know what option {0} does.'.format(sys.argv[1])
        if not raw_input('Continue (y/n)?').lower() in ('y', 'yes'):
            sys.exit(0)

setup(
    options = {'py2exe': {'bundle_files': 1,
                          'compressed': True,
                          'optimize': 1}},
    windows = [{'script': 'serial_comunicator.py'}],
    zipfile = None,
)
