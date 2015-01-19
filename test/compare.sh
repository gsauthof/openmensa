#!/bin/bash

set -e
set -u

DIFF=diff

EXE="$1"
IN="$2"
OUT="$3"
REF="$4"

#set -x

"$EXE" "$IN" > "$OUT"
"$DIFF" -uw "$REF" "$OUT"


