#!/bin/bash
. "$(dirname "${0}")/lib.sh"
cd "$SRC" || exit 1
export LC_ALL=C
rc=0
ok()   { echo "  OK   $1"; }
fail() { echo "  FAIL $1"; rc=1; }

EXP1=670fd8c2df070c60b045671b8b24ff02
EXP2=63ebaa5281d37a1e2c9addd99b872448

echo "[1] Radius: create_algo_MD5_radius contra openssl md5"
{ echo '#include <cstring>'; echo '#include <openssl/evp.h>'
  sed -n '/^int create_algo_MD5_radius/,/^}/p' library-crypto/CryptExternalMethods.cpp; } > /tmp/radius_only.cpp
if g++ -g -Wall -fsanitize=address,undefined -o /tmp/test_radius \
      "$HERE/unit/test_radius.cpp" /tmp/radius_only.cpp -lcrypto 2>/tmp/radius_build.log; then
  got=$(/tmp/test_radius 2>&1 | sed -n 's/^rc=0 //p')
  exp=$(printf '%s%s' secreto hola-mundo | openssl md5 | awk '{print $NF}')
  if [ -n "$got" ] && [ "$got" = "$exp" ]; then ok "hash coincide ($got)"; else fail "obtenido=[$got] esperado=[$exp]"; fi
else
  fail "no compila (ver /tmp/radius_build.log)"
fi

echo "[2] Digest: hashes y cabecera demasiado larga"
if gcc -g -Wall -fsanitize=address,undefined -I library-crypto -o /tmp/test_auth "$HERE/unit/test_auth.c" \
      library-crypto/auth.c library-crypto/milenage.c library-crypto/rijndael.c -lcrypto 2>/tmp/auth_build.log; then
  r1=$(/tmp/test_auth | grep -o 'response="[0-9a-f]*"' | sed -n 1p | cut -d'"' -f2)
  r2=$(/tmp/test_auth | grep -o 'response="[0-9a-f]*"' | sed -n 2p | cut -d'"' -f2)
  if [ "$r1" = "$EXP1" ] && [ "$r2" = "$EXP2" ]; then ok "los dos response coinciden"; else fail "response: [$r1] [$r2]"; fi
  l=$(ASAN_OPTIONS=detect_leaks=0 /tmp/test_auth long 2>&1 | head -n 1)
  case "$l" in
    "long: rc=0"*) ok "realm largo rechazado sin desbordar" ;;
    *) fail "long: $l" ;;
  esac
else
  fail "no compila (ver /tmp/auth_build.log)"
fi

echo "[3] Compilacion con -Werror (sin y con plugins OpenSSL)"
for d in build-werror build-werror-ssl; do
  cmake --build $d -j$(nproc) -- -k 0 > /tmp/$d.log 2>&1
  n=$(grep -c 'FAILED:' /tmp/$d.log)
  if [ "$n" = 0 ]; then ok "$d sin fallos"; else fail "$d: $n destinos fallidos (ver /tmp/$d.log)"; fi
done
for f in lib_crypto.so libtrans_iptls.so; do
  if [ -f build-werror-ssl/bin/$f ]; then ok "$f generado"; else fail "$f no existe"; fi
done

echo "[4] Referencia Diameter (no ejercita crypto, solo detecta regresiones)"
"$HERE/verify.sh" "$SRC/build-werror-ssl/bin" || fail "verify.sh"

echo
if [ $rc = 0 ]; then echo "TODO OK"; else echo "HAY FALLOS"; fi
exit $rc
