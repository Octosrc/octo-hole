import os
import re
import sys
import math

HASH_BYTES = 5
MASK = (1 << (HASH_BYTES * 8)) - 1
FNV_OFFSET = 0xcbf29ce484222325
FNV_PRIME = 0x100000001b3
U64 = (1 << 64) - 1

DEFAULT_INPUT = 'list.txt'
HERE = os.path.dirname(os.path.abspath(__file__))

ADG = re.compile(r'^(@@)?\|\|([a-z0-9._-]+)\^?', re.I)


def fnv(b: bytes) -> int:
    h = FNV_OFFSET
    for c in b:
        h = ((h ^ c) * FNV_PRIME) & U64
    return h & MASK


def norm(d: str) -> str:
    return d.strip().lower().lstrip('*').lstrip('.').rstrip('.')


def host_of(line: str) -> str | None:
    m = ADG.match(line)
    if m:
        return m.group(2)

    s = line.strip()
    if '://' in s:
        s = s.split('://', 1)[1]
    s = s.split('/', 1)[0]
    s = s.split('?', 1)[0]
    s = s.split('#', 1)[0]
    s = s.rsplit('@', 1)[-1]
    s = s.split(':', 1)[0]
    if not s or ' ' in s:
        return None
    return s


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    src = args[0] if args else DEFAULT_INPUT
    if not os.path.isabs(src):
        src = os.path.join(HERE, src)

    if not os.path.exists(src):
        print(f'!! input file not found: {src}', file=sys.stderr)
        print(f'   put a {DEFAULT_INPUT} next to this script and run it again.', file=sys.stderr)
        sys.exit(1)

    out = os.path.splitext(src)[0] + '.bin'

    domains, allow = set(), set()
    skipped = 0

    with open(src, errors='ignore') as f:
        for raw in f:
            line = raw.strip()
            if not line or line[0] in '#!;/[':
                continue
            is_allow = line.startswith('@@')
            if is_allow:
                line = line[2:]
            parts = line.split()
            if len(parts) >= 2 and parts[0] in ('0.0.0.0', '127.0.0.1', '::1', '::'):
                line = parts[1]
            d = host_of(line)
            d = norm(d) if d else None
            if not d or '.' not in d or ' ' in d:
                skipped += 1
                continue
            (allow if is_allow else domains).add(d)

    if allow:
        before = len(domains)
        domains -= allow
        print(f'allowlisted    : {before - len(domains):,} removed')

    hashes = sorted({fnv(d.encode()) for d in domains})
    with open(out, 'wb') as f:
        for h in hashes:
            f.write(h.to_bytes(HASH_BYTES, 'little'))

    size = len(hashes) * HASH_BYTES
    print(f'input file     : {os.path.basename(src)}')
    print(f'domains        : {len(domains):,}')
    if skipped:
        print(f'skipped lines  : {skipped:,}')
    print(f'hash entries   : {len(hashes):,}  ({HASH_BYTES}-byte / {HASH_BYTES * 8}-bit)')
    print(f'flash blob     : {size:,} bytes  -> {os.path.basename(out)}')
    print(f'lookup         : ~{math.ceil(math.log2(max(len(hashes), 2)))} reads/query')


if __name__ == '__main__':
    main()
