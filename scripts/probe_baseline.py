"""Read-only bounded baseline. Never acquires a control session."""
import argparse
import json
import time
import urllib.request
from pathlib import Path


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--url', required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--samples', type=int, default=60)
    args = p.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))
    with args.output.open('x', encoding='utf-8') as log:
        for i in range(args.samples):
            start = time.monotonic()
            row = dict(sample=i, utc=time.time())
            try:
                with opener.open(args.url.rstrip('/') + '/api/v1/status', timeout=2) as r:
                    row['status'] = json.load(r)
            except Exception as e:
                row['error'] = str(e)
            row['latency_ms'] = (time.monotonic() - start) * 1000
            log.write(json.dumps(row) + '\n')
            log.flush()
            time.sleep(max(0, 1 - (time.monotonic() - start)))


if __name__ == '__main__':
    main()
