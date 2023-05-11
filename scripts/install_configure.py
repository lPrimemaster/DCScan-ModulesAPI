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

# Return lines to skip on the block (or not) [start, stop]
def checkBlockActive(content: list[str], compdef: list[str]):
    if (content[0].startswith('#ifdef') or content[0].startswith('#ifndef')) and not content[1].startswith('#define'):
        if content[0].startswith('#ifdef'):
            if content[0].split()[-1].strip() in compdef:
                rgroup = r'\2'
            else:
                rgroup = r'\3'
        else:
            if content[0].split()[-1].strip() in compdef:
                rgroup = r'\3'
            else:
                rgroup = r'\2'
        
        new_str = re.sub('#ifn?def +(.*?)\n(.*?)#else(.*?)(#endif.*?\n)', rgroup, ''.join(content), flags=re.DOTALL)

        return new_str.splitlines(keepends=True)
    return []
        
        

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
        i = 0
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
