#!/usr/bin/env bash
# Test harness for ./encrypt without needing ./decrypt

set -euo pipefail

KEYGEN=${KEYGEN:-./keygen}
ENCRYPT=${ENCRYPT:-./encrypt}
BITS=${BITS:-1024}

rebuild=${REBUILD:-1}
if [[ -f Makefile ]]; then
  if [[ "${rebuild}" == "1" ]]; then
    make clean && make keygen encrypt
  else
    make keygen encrypt
  fi
fi

# Fresh keys (deterministic seed for reproducibility)
$KEYGEN -s 1 >/dev/null

# Get n (hex) and compute bit-length and k per spec
nhex=$(sed -n '1p' ss.pub | tr -d '\r\n')

# compute bit length from hex (handle leading nibble)
digits=${#nhex}
first=${nhex:0:1}
case "$first" in
  [8-9a-f]) lead=0 ;;
  [4-7])    lead=1 ;;
  [2-3])    lead=2 ;;
  1)        lead=3 ;;
  *) echo "bad first hex nibble"; exit 1 ;;
esac
nbits=$(( 4*digits - lead ))
sqrt_bits=$(( (nbits + 1) / 2 ))         # ceil(nbits/2)
k=$(( (sqrt_bits - 1) / 8 ))             # floor((ceil(log2(sqrt(n))) - 1)/8)
payload=$(( k - 1 ))
echo "n has $nbits bits; k=$k; payload size per block: $payload bytes"

# helper: hex(a) < hex(b) ? (strict)
hex_lt() {
  local a="$1" b="$2"
  # strip leading zeros for clean comparison
  a="${a##+(0)}"; [[ -z "$a" ]] && a=0
  b="${b##+(0)}"; [[ -z "$b" ]] && b=0
  if (( ${#a} != ${#b} )); then
    (( ${#a} < ${#b} ))
  else
    [[ "$a" < "$b" ]]   # works for 0-9a-f
  fi
}

tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT

# 1) stdin->stdout basic: small text
echo -n "hello world" | $ENCRYPT > "$tmpdir/e1.hex"
# all lines are lowercase hex, and each < n
if ! awk 'tolower($0)==$0 && $0 ~ /^[0-9a-f]+$/' "$tmpdir/e1.hex" | diff -q - "$tmpdir/e1.hex" >/dev/null; then
  echo "FAIL: non-lowercase or non-hex line(s) in stdout case"; exit 1
fi
while IFS= read -r line; do
  if ! hex_lt "$line" "$nhex"; then echo "FAIL: ciphertext >= n"; exit 1; fi
  if [[ "$line" == "0" || "$line" == "1" ]]; then echo "FAIL: ciphertext 0/1"; exit 1; fi
done < "$tmpdir/e1.hex"
echo "ok: stdin/stdout basic"

# 2) empty input => empty output
: > "$tmpdir/empty"
$ENCRYPT -i "$tmpdir/empty" -o "$tmpdir/empty.hex" >/dev/null
test ! -s "$tmpdir/empty.hex" && echo "ok: empty input produces empty output"

# 3) block-boundary sizes
make_data() { head -c "$1" </dev/urandom > "$tmpdir/in_$1.bin"; }

sizes=()
if (( payload > 3 )); then
  sizes+=( $((payload-1)) $payload $((payload+1)) $((3*payload+5)) $((7*payload+1)) )
else
  sizes+=( 1 2 3 10 100 )
fi

for L in "${sizes[@]}"; do
  make_data "$L"
  $ENCRYPT -i "$tmpdir/in_$L.bin" -o "$tmpdir/out_$L.hex" -n ss.pub >/dev/null

  # expected blocks = ceil(L / (k-1)) = ceil(L / payload)
  if (( payload > 0 )); then
    blocks=$(( (L + payload - 1) / payload ))
  else
    echo "Computed payload <= 0; something is wrong"; exit 1
  fi

  got=$(wc -l < "$tmpdir/out_$L.hex" | tr -d ' ')
  if [[ "$got" != "$blocks" ]]; then
    echo "FAIL: size $L expected $blocks lines, got $got"; exit 1
  fi

  # per-line checks again (hex, lowercase, < n, not 0/1)
  if ! awk 'tolower($0)==$0 && $0 ~ /^[0-9a-f]+$/' "$tmpdir/out_$L.hex" | diff -q - "$tmpdir/out_$L.hex" >/dev/null; then
    echo "FAIL: non-lowercase or non-hex lines for L=$L"; exit 1
  fi
  while IFS= read -r line; do
    if ! hex_lt "$line" "$nhex"; then echo "FAIL: L=$L ciphertext >= n"; exit 1; fi
    if [[ "$line" == "0" || "$line" == "1" ]]; then echo "FAIL: L=$L ciphertext 0/1"; exit 1; fi
  done < "$tmpdir/out_$L.hex"

  echo "ok: size $L → $got line(s)"
done

# 4) bad pubkey path
if $ENCRYPT -n does_not_exist.pub </dev/null >/dev/null 2>&1; then
  echo "FAIL: missing pubkey should exit non-zero"; exit 1
fi
echo "ok: missing pubkey handled"

echo "All encrypt checks passed ✅"
