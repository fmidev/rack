#!/usr/bin/env python3
"""Parse a Quay.io build log JSON and print a readable summary.

Usage:
    python3 scripts/quay-log.py <log.json> [--errors-only]

The JSON file is downloaded from the Quay.io build page (Actions → Download build logs).
"""

import json
import re
import sys

# ANSI colour codes from Docker build output
ANSI_RE = re.compile(r'\x1b\[[0-9;]*m')

ERROR_KEYWORDS = re.compile(
    r'\b(error|fail|fatal|cannot|could not|no such|not found|permission denied|timed out|refused)\b',
    re.IGNORECASE
)

PHASE_LABELS = {
    'build-scheduled': '=== BUILD SCHEDULED ===',
    'unpacking':       '=== UNPACKING SOURCE ===',
    'pulling':         '=== PULLING BASE IMAGE ===',
    'building':        '=== BUILDING ===',
    'complete':        '=== COMPLETE ===',
    'error':           '=== ERROR ===',
}


def strip_ansi(text: str) -> str:
    return ANSI_RE.sub('', text)


def main():
    errors_only = '--errors-only' in sys.argv
    paths = [a for a in sys.argv[1:] if not a.startswith('--')]

    if not paths:
        print(__doc__)
        sys.exit(1)

    for path in paths:
        with open(path) as f:
            data = json.load(f)

        logs = data.get('logs', data) if isinstance(data, dict) else data

        print(f"\n{'=' * 60}")
        print(f"Log: {path}  ({len(logs)} entries)")
        print('=' * 60)

        step = None
        for entry in logs:
            if not isinstance(entry, dict):
                continue

            etype = entry.get('type', '')
            msg = strip_ansi(entry.get('message', ''))
            dt = entry.get('data', {}).get('datetime', '')

            if etype == 'phase':
                label = PHASE_LABELS.get(msg, f'=== {msg.upper()} ===')
                print(f'\n{label}  [{dt}]')
                step = None
                continue

            if etype == 'command':
                step = msg
                if not errors_only:
                    print(f'\n>> {msg}')
                continue

            # Regular log line
            is_error = bool(ERROR_KEYWORDS.search(msg))

            if errors_only:
                if is_error:
                    if step:
                        print(f'\n>> {step}')
                        step = None  # print step header once
                    print(f'  ! {msg}')
            else:
                prefix = '  ! ' if is_error else '    '
                for line in msg.splitlines():
                    if line.strip():
                        print(f'{prefix}{line}')


if __name__ == '__main__':
    main()
