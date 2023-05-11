import re

# Return cleaned lines to skip on the block (or not)
def checkBlockActive(content: list[str], compdef: list[str]):
    if (content[0].startswith('#ifdef') or content[0].startswith('#ifndef')) and content[0].split()[-1].strip().startswith('NO'):
        rgroup = r'\2'
        if content[0].startswith('#ifdef'):
            if not (content[0].split()[-1].strip() in compdef):
                rgroup = r''
        else:
            if content[0].split()[-1].strip() in compdef:
                rgroup = r''
        
        new_str = re.sub('#ifn?def +(.*?)\n(.*?)(#endif.*?\n)', rgroup, ''.join(content), flags=re.DOTALL)

        return new_str.splitlines(keepends=True)
    return []