# ============================================================== #
# Configures public header files with preprocessor conditionals. #
# Cesar Godinho 2023/05/11                                       #
# ============================================================== #

import pprint
import os
import fnmatch
from pathlib import Path
import re
import sys
from common import checkBlockActive

if __name__ == '__main__':
    compdef = sys.argv[1].split(';')
    build_dir = sys.argv[2]
    pp = pprint.PrettyPrinter(indent=0)
    curr_dir = os.getcwd()

    filenames = []
    for root, dirs, files in os.walk(curr_dir):
        for file in files:
            if fnmatch.fnmatch(file, '*.h') and not any(x in root for x in ['build', 'install', 'submodules', 'tests', 'config']):
                filenames.append(os.path.join(root, file))

    print('Configuring files...')
    pp.pprint(filenames)
    print('\n\n')

    for fnm in filenames:
        if 'internal.h' in fnm: continue
        with open(fnm, 'r') as f:
            lst = f.readlines()
        mod = False
        i = 1 # Ignore header guards
        while i < len(lst):
            nline = lst[i].lstrip().replace('\n', '')
            i += 1
            if not nline: continue

            plist = checkBlockActive(lst[i-1:], compdef)
            if plist:
                mod = True
                lst[i-1:] = plist

        # Save lst to temp directory in build
        match = re.match(".*(DCS_.+?include.)(.+\.h)", fnm)
        gen_dir = f'{build_dir}/gen/{match.group(1)}'
        Path(gen_dir).mkdir(parents=True, exist_ok=True)
        with open(f'{gen_dir}/{match.group(2)}', 'w') as w:
            w.writelines(lst)
        if mod:
            print('Modified file: ' + ''.join(match.groups()))
