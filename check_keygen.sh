#!/usr/bin/env bash
# Simple test harness for ./keygen
# Usage: ./check_keygen.sh [bits]
# Env vars:
#   REBUILD=1  (default) -> runs `make clean && make keygen`
#   REBUILD=0             -> just runs `make keygen`
#   KEYGEN=./keygen       -> path to keygen

set -euo pipefail

BITS="${1:-1024}"
REBUILD="${REBUILD:-1}"
KEYGEN="${KEYGEN:-./keygen}"

# --- helpers ---
perm_linux() { stat -c '%a' "$1"; }
is_hex() { grep -Eq '^[0-9a-f]+$'; }

# --- build ---
if [ "$REBUILD" = "1" ] && [ -f Makefile ]; then
  echo "== Rebuild =="
  make clean && make keygen
elif [ -f Makefile ]; then
  echo "== Build (no clean) =="
  make keygen
fi
[ -x "$KEYGEN" ] || { echo "keygen not found/executable at $KEYGEN"; exit 1; }

# clean any previous outputs we might collide with
rm -f ss.pub ss.priv a.pub a.priv b.pub b.priv c.pub c.priv bits.pub bits.priv

echo "== Default run (verbose) =="
"$KEYGEN" -v

echo "== Files exist =="
test -f ss.pub && echo "  ok: ss.pub"
test -f ss.priv && echo "  ok: ss.priv"

echo "== Private key perms are 0600 =="
perm="$(perm_linux ss.priv)"
if [ "$perm" != "600" ]; then
  echo "  FAIL: ss.priv perms are $perm (expected 600)"; exit 1
fi
echo "  ok: perms 600"

echo "== Deterministic with same seed =="
"$KEYGEN" -n a.pub -d a.priv -s 123 >/dev/null
"$KEYGEN" -n b.pub -d b.priv -s 123 >/dev/null
if ! cmp -s a.pub b.pub || ! cmp -s a.priv b.priv; then
  echo "  FAIL: outputs differ for same seed"; exit 1
fi
echo "  ok: same-seed identical"

echo "== Different with different seed =="
"$KEYGEN" -n c.pub -d c.priv -s 124 >/dev/null
if cmp -s a.pub c.pub && cmp -s a.priv c.priv; then
  echo "  FAIL: outputs identical for different seeds"; exit 1
fi
echo "  ok: different-seed differs"

echo "== Public file format quick checks =="
pub_line1="$(sed -n '1p' ss.pub | tr -d '\r\n')"
pub_line2="$(sed -n '2p' ss.pub | tr -d '\r\n')"
if ! printf "%s" "$pub_line1" | is_hex; then
  echo "  FAIL: first line of ss.pub is not lowercase hex"; exit 1
fi
expected_user="${USER:-$(whoami)}"
if [ "$pub_line2" != "$expected_user" ]; then
  echo "  WARN: username mismatch (file:'$pub_line2' vs env:'$expected_user')"
else
  echo "  ok: username line"
fi

echo "== Private file format quick checks =="
lines_priv="$(wc -l < ss.priv | tr -d ' ')"
if [ "$lines_priv" -ne 2 ]; then
  echo "  FAIL: ss.priv should have exactly 2 lines"; exit 1
fi
if ! sed -n '1,2p' ss.priv | is_hex; then
  echo "  FAIL: ss.priv lines must be lowercase hex"; exit 1
fi
echo "  ok: private file format"

echo "== -b bits sanity =="
"$KEYGEN" -n bits.pub -d bits.priv -b "$BITS" >/dev/null
hex_digits="$(sed -n '1p' bits.pub | tr -d '\r\n' | wc -c | tr -d ' ')"
need=$(( (BITS + 3) / 4 ))   # ceil(bits/4)
if [ "$hex_digits" -lt "$need" ]; then
  echo "  FAIL: n hex length $hex_digits < ceil($BITS/4)=$need"; exit 1
fi
echo "  ok: n length consistent with requested bits"

echo "== Option smoke tests (-n/-d/-i/-s) =="
rm -f foo.pub bar.priv
"$KEYGEN" -n foo.pub -d bar.priv -i 5 -s 42 >/dev/null
test -f foo.pub && test -f bar.priv && echo "  ok: custom paths & options"

echo "== Negative input tests =="
if "$KEYGEN" -b notanumber >/dev/null 2>&1; then
  echo "  FAIL: -b notanumber should fail"; exit 1
fi
if "$KEYGEN" -s -1 >/dev/null 2>&1; then
  echo "  FAIL: -s -1 should fail"; exit 1
fi

echo "All keygen checks passed ✅"
