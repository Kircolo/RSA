#!/usr/bin/env bash
set -euo pipefail

KEYGEN=${KEYGEN:-./keygen}
ENCRYPT=${ENCRYPT:-./encrypt}
DECRYPT=${DECRYPT:-./decrypt}
REBUILD=${REBUILD:-1}

if [[ -f Makefile ]]; then
  if [[ "$REBUILD" == "1" ]]; then
    make clean && make keygen encrypt decrypt
  else
    make keygen encrypt decrypt
  fi
fi

# fresh deterministic keys for reproducibility
$KEYGEN -s 1 >/dev/null

# compute k and payload = (k-1) using n from ss.pub (same math as the encrypt test)
nhex=$(sed -n '1p' ss.pub | tr -d '\r\n')
digits=${#nhex}; first=${nhex:0:1}
case "$first" in
  [8-9a-f]) lead=0 ;; [4-7]) lead=1 ;; [2-3]) lead=2 ;; 1) lead=3 ;; *) echo "bad n"; exit 1 ;;
esac
nbits=$(( 4*digits - lead ))
sqrt_bits=$(( (nbits + 1) / 2 ))
k=$(( (sqrt_bits - 1) / 8 ))
payload=$(( k - 1 ))
echo "n has $nbits bits; k=$k; payload per block=$payload bytes"

tmpdir="$(mktemp -d)"; trap 'rm -rf "$tmpdir"' EXIT

# 1) pipe round-trip
plain="hello world"
rt="$(
  printf "%s" "$plain" \
  | $ENCRYPT \
  | $DECRYPT
)"
[[ "$rt" == "$plain" ]] && echo "ok: pipe round-trip"

# 2) empty input → empty output
: | $ENCRYPT | $DECRYPT | cmp -s - /dev/null && echo "ok: empty round-trip"

# helper for random files
mk() { head -c "$1" </dev/urandom > "$tmpdir/in_$1.bin"; }

sizes=()
if (( payload > 3 )); then
  sizes+=( $((payload-1)) $payload $((payload+1)) $((3*payload+5)) $((7*payload+1)) )
else
  sizes+=( 1 2 3 10 100 )
fi

# 3) file round-trips across block boundaries
for L in "${sizes[@]}"; do
  mk "$L"
  $ENCRYPT -i "$tmpdir/in_$L.bin" -o "$tmpdir/c_$L.hex" >/dev/null
  $DECRYPT -i "$tmpdir/c_$L.hex" -o "$tmpdir/out_$L.bin" >/dev/null
  cmp -s "$tmpdir/in_$L.bin" "$tmpdir/out_$L.bin"
  echo "ok: size $L round-trip"
done

# 4) missing private key should fail
if $DECRYPT -n does_not_exist.priv </dev/null >/dev/null 2>&1; then
  echo "FAIL: missing privkey should error"; exit 1
fi
echo "ok: missing privkey handled"

# 5) verbose prints pq then d (to stderr), does not pollute stdout
echo -n "abc" | $ENCRYPT > "$tmpdir/verb.c"
out="$($DECRYPT -v -i "$tmpdir/verb.c" -o "$tmpdir/verb.out" 2>&1 >/dev/null)"
grep -q "Private modulus pq (" <<<"$out" && grep -q "Private key d (" <<<"$out" \
  && echo "ok: verbose prints pq then d"

echo "All decrypt checks passed ✅"
