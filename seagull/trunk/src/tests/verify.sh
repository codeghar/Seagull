#!/bin/bash
# uso: verify.sh <dir-binarios> [dir-salida]
# Corre Diameter TCP y compara contra la referencia gnu++98 (tests/expected)
. "$(dirname "${0}")/lib.sh"
OUT=${2:-$WORK/out/verify}
"$HERE/run-diameter.sh" "${1}" "$OUT" | grep -E 'terminó|NO|AVISO|uso:|No se pudo|no se abrió|errores E en los logs|RESULTADO|^ +[0-9]+ '
[ "${PIPESTATUS[0]}" -eq 0 ] || { echo "ERROR: la corrida falló, no se compara"; exit 1; }
[ -f "$OUT/env/logs/client-protocol-stat.diameter-v1.csv" ] || { echo "ERROR: no se generó el CSV del cliente"; exit 1; }
sar=$("$HERE/resumen.sh" "$OUT" client | cut -d';' -f10)
echo "SAR enviados (esperado 100): $sar"
rc=0
for lado in client server; do
  if diff -q "$EXPECTED/$lado.txt" <("$HERE/resumen.sh" "$OUT" $lado) >/dev/null; then
    echo "  $lado: IGUAL"
  else
    echo "  $lado: DIFERENTE"; rc=1
  fi
done
exit $rc
