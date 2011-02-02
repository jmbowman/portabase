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
    f = open(path, 'r')
    content = unicode(f.read(), 'utf-8')
    f.close()
    f = open(path, 'w')
    f.write(content.replace(u'_static', u'../_static').encode('utf-8'))
    f.close()
