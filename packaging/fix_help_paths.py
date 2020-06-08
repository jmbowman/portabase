# Script to replace the paths to static assets in the generated help files so
# that one copy is shared by all languages.  Done with sed on UNIX-based
# platforms, so this is mainly for Windows.

from os import listdir
from os.path import join
from sys import argv

dir = argv[1]
for file in listdir(dir):
    if len(file) < 6 or file[-5:] != '.html':
        continue
    path = join(dir, file)
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    content = content.replace('_static/translations', 'translations')
    content = content.replace('_static', '../_static')
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
