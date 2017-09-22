"""Common testing functions."""


def fix_func_enum(globs):
    """Remove tuples from func_* globals."""
    for glob in globs.keys():
        if glob.startswith("func_"):
            globs[glob] = globs[glob][0]
