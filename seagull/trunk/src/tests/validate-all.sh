#!/bin/bash
# uso: [STD_LOOP=1] validate-all.sh
. "$(dirname "${0}")/lib.sh"
D=$WORK/validation/$(date +%Y%m%d-%H%M)
mkdir -p "$D"; export LC_ALL=C
export LSAN_OPTIONS=suppressions=$HERE/lsan.supp
cd "$SRC" || exit 1
rc=0
ok()    { echo "  OK    $1"; }
fail()  { echo "  FAIL  $1"; rc=1; }
warn()  { echo "  AVISO $1"; }
stage() { echo; echo "== $1"; }

san() {   # $1 = directorio de salida, $2 = etiqueta
  local hits
  hits=$(grep -lE 'AddressSanitizer|LeakSanitizer|runtime error' "$1"/env/logs/*.out "$1"/env/logs/*.log 2>/dev/null)
  if [ -z "$hits" ]; then ok "$2: sin informes de sanitizers"
  else fail "$2: informes en: $hits"; grep -h SUMMARY $hits | sort | uniq -c; fi
}

build() { # nombre, opciones de cmake...
  local name=$1; shift
  rm -rf "$name"
  if ! cmake -S . -B "$name" -G Ninja "$@" > "$D/$name.configure.log" 2>&1; then
    fail "$name: cmake fallo (ver $D/$name.configure.log)"; return
  fi
  cmake --build "$name" -j$(nproc) -- -k 0 > "$D/$name.build.log" 2>&1
  local f e w
  f=$(grep -c 'FAILED:' "$D/$name.build.log")
  e=$(grep -c ': error:' "$D/$name.build.log")
  w=$(grep -c '\[-W' "$D/$name.build.log")
  if [ "$f" = 0 ] && [ "$e" = 0 ] && [ "$w" = 0 ]; then ok "$name: 0 fallos, 0 errores, 0 advertencias"
  else fail "$name: FAILED=$f errores=$e advertencias=$w (ver $D/$name.build.log)"; fi
}

stage "0. Preflight"
pkill seagull 2>/dev/null; sleep 1
if ss -ltn | grep -q ':3868 '; then fail "puerto 3868 ocupado"; else ok "puerto 3868 libre"; fi
for f in "$HERE/unit/test_auth.c" "$HERE/unit/test_radius.cpp" "$HERE/unit/test_bufferutils.cpp" "$HERE/unit/build-test-bufferutils.sh" "$CERTS/cert.pem" "$CERTS/key.pem" "$HERE/run-diameter.sh" "$HERE/run-diameter-tls.sh" "$HERE/run-diameter-sctp.sh" "$HERE/run-diameter-load.sh" "$HERE/verify.sh" "$HERE/resumen.sh" "$HERE/validate-crypto.sh" "$EXPECTED/client.txt" "$EXPECTED/server.txt"; do
  if [ -f "$f" ]; then ok "existe $(basename "$f")"; else fail "falta $f"; fi
done
echo "  git HEAD: $(git log --oneline -n 1)"
n=$(git status --short | wc -l)
if [ "$n" = 0 ]; then ok "arbol de git limpio"; else warn "$n archivos sin confirmar: se prueba codigo sin commit"; fi

stage "1. Compilacion desde cero"
build build-werror     -DCMAKE_BUILD_TYPE=Debug -DSEAGULL_WERROR=ON
build build-werror-ssl -DCMAKE_BUILD_TYPE=Debug -DSEAGULL_WERROR=ON -DSEAGULL_BUILD_CRYPTO=ON -DSEAGULL_BUILD_TLS=ON -DSEAGULL_BUILD_SCTP=ON
build build-asan       -DCMAKE_BUILD_TYPE=Debug -DSEAGULL_SANITIZE=ON -DSEAGULL_BUILD_CRYPTO=ON -DSEAGULL_BUILD_TLS=ON
build build-release    -DCMAKE_BUILD_TYPE=Release -DSEAGULL_BUILD_CRYPTO=ON -DSEAGULL_BUILD_TLS=ON -DSEAGULL_BUILD_SCTP=ON
if [ -n "$STD_LOOP" ]; then
  for s in 98 11 14 17 20 23; do
    build build-std-$s -DCMAKE_BUILD_TYPE=Debug -DSEAGULL_CXX_STD=$s
    echo "       $(grep -m1 -o 'std=[^ ]*' build-std-$s/compile_commands.json)"
  done
fi

stage "2. Crypto: Radius y Digest contra openssl md5, -Werror, plugins"
"$HERE/validate-crypto.sh" > "$D/crypto.log" 2>&1
grep -E 'OK|FAIL' "$D/crypto.log"
if grep -q 'TODO OK' "$D/crypto.log"; then ok "validate-crypto.sh"; else fail "validate-crypto.sh (ver $D/crypto.log)"; fi
"$HERE/unit/build-test-bufferutils.sh" > "$D/bufferutils.log" 2>&1
if grep -q 'test_bufferutils: OK' "$D/bufferutils.log"; then ok "unit/test_bufferutils (ASan/UBSan)"; else fail "unit/test_bufferutils (ver $D/bufferutils.log)"; fi

stage "3. Diameter TCP con ASan, comparado con la referencia gnu++98"
"$HERE/verify.sh" "$SRC/build-asan/bin" "$D/out-tcp-asan" > "$D/tcp-asan.log" 2>&1
cat "$D/tcp-asan.log"
if grep -q 'client: IGUAL' "$D/tcp-asan.log" && grep -q 'server: IGUAL' "$D/tcp-asan.log"; then ok "contadores IGUAL"; else fail "contadores distintos de la referencia"; fi
san "$D/out-tcp-asan" "TCP+ASan"

stage "4. Diameter TLS con ASan"
PROBE=1 "$HERE/run-diameter-tls.sh" "$SRC/build-asan/bin" "$D/out-tls-asan" > "$D/tls-asan.log" 2>&1
grep -E 'Protocol|Cipher|SAR enviados|IGUAL|DIFERENTE|termin' "$D/tls-asan.log"
if grep -q 'Protocol: TLSv' "$D/tls-asan.log" && grep -q 'client: IGUAL' "$D/tls-asan.log" && grep -q 'server: IGUAL' "$D/tls-asan.log"; then
  ok "TLS negocia y los contadores coinciden"; else fail "TLS (ver $D/tls-asan.log)"; fi
san "$D/out-tls-asan" "TLS+ASan"

stage "5. Carga (primera ejecucion del script de carga)"
B=$SRC/build-release/bin
for m in tcp tls; do
  MODE=$m RATE=200 CALLS=2000 "$HERE/run-diameter-load.sh" "$B" "$D/out-load-$m" > "$D/load-$m.log" 2>&1
  grep -E 'cliente:|servidor:|tasa|RESULTADO' "$D/load-$m.log"
  if grep -q 'RESULTADO: OK' "$D/load-$m.log"; then ok "carga $m"; else fail "carga $m (ver $D/load-$m.log)"; fi
done
MODE=tls RATE=20 CALLS=200 "$HERE/run-diameter-load.sh" "$SRC/build-asan/bin" "$D/out-load-tls-asan" > "$D/load-tls-asan.log" 2>&1
grep -E 'RESULTADO' "$D/load-tls-asan.log"
if grep -q 'RESULTADO: OK' "$D/load-tls-asan.log"; then ok "carga tls con ASan"; else fail "carga tls con ASan"; fi
san "$D/out-load-tls-asan" "carga TLS+ASan"

stage "5b. Diameter SCTP nativo (opcional: requiere el modulo sctp)"
if checksctp >/dev/null 2>&1; then
  "$HERE/run-diameter-sctp.sh" "$SRC/build-werror-ssl/bin" "$D/out-sctp" > "$D/sctp.log" 2>&1
  if [ -f "$D/out-sctp/env/logs/client-protocol-stat.diameter-v1.csv" ] && diff -q "$EXPECTED/client.txt" <("$HERE/resumen.sh" "$D/out-sctp" client) >/dev/null && diff -q "$EXPECTED/server.txt" <("$HERE/resumen.sh" "$D/out-sctp" server) >/dev/null; then ok "SCTP: contadores IGUAL"; else fail "SCTP (ver $D/sctp.log)"; fi
else
  warn "SCTP omitido: modulo sctp no cargado (sudo modprobe sctp)"
fi

stage "6. Resumen"
if pgrep -x seagull >/dev/null; then warn "queda algun seagull vivo"; pkill seagull; fi
echo "  logs en: $D"
if [ $rc = 0 ]; then echo "  VALIDACION COMPLETA: TODO OK"; else echo "  VALIDACION COMPLETA: HAY FALLOS"; fi
exit $rc
