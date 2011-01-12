#!/usr/bin/env python

# Use this script instead of running lupdate directly in order to prune some
# phrases from the Qt translation files that lupdate insists on adding to the
# PortaBase translation files (because they're used directly in the code).

from os import listdir, system
from os.path import join
import re

system('lupdate portabase.pro')
pattern = re.compile(r'<context>\s*<name>QFileSystemModel((?!</context>).)+</context>\s*', re.S)
dir = join('resources', 'translations')
for path in listdir(dir):
    if not path[:4] == 'port' or not path[-3:] == '.ts':
        continue
    file = open(join(dir, path), 'r')
    content = ''.join(file.readlines())
    file.close()
    content = pattern.sub('', content)
    file = open(join(dir, path), 'w')
    file.write(content)
    file.close()
