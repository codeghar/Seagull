#!/bin/bash
# uso: build-test-bufferutils.sh [dir-build-asan]   (por defecto <src>/build-asan)
# Compila test_bufferutils.cpp + BufferUtils.cpp con ASan/UBSan y enlaza el resto
# de dependencias desde los objetos de build-asan (sin main ni BufferUtils.cpp.o).
set -e
HERE=$(dirname "$(realpath "${0}")")
SRC=$(realpath "$HERE/../..")
BUILD=$(realpath "${1:-$SRC/build-asan}")
OBJDIR=$BUILD/CMakeFiles/seagull.dir
[ -d "$OBJDIR" ] || { echo "no existe $OBJDIR (configura antes build-asan)"; exit 1; }
cmake --build "$BUILD" > /dev/null
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT
MAINOBJ=$(nm -A --defined-only $(find "$OBJDIR" -name '*.o') 2>/dev/null | grep -E ' T main$' | cut -d: -f1)
find "$OBJDIR" -name '*.o' ! -path "$MAINOBJ" ! -name 'BufferUtils.cpp.o' > "$TMP/objs.txt"
ar rcs "$TMP/libdeps.a" $(cat "$TMP/objs.txt")
INC=$(find "$SRC" -mindepth 1 -maxdepth 1 -type d ! -name 'build-*' ! -name tests -printf '-I%p ')
c++ -std=gnu++23 -g -O1 -fsanitize=address,undefined -fno-sanitize-recover=all \
    -fno-omit-frame-pointer -D_GNU_SOURCE -D_REENTRANT $INC -I"$BUILD" \
    "$HERE/test_bufferutils.cpp" "$SRC/generator-common/BufferUtils.cpp" \
    "$TMP/libdeps.a" -lpthread -ldl -o "$TMP/test_bufferutils"
"$TMP/test_bufferutils"
