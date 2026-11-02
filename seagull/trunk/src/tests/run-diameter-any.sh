#!/bin/bash
# uso: run-diameter-any.sh <tcp|tls|sctp> <dir-binarios> <dir-salida> [clave] [passwd]   (TLS: PROBE=1 prueba s_client)
. "$(dirname "${0}")/lib.sh"
MODE=${1}; BIN=$(realpath "${2}" 2>/dev/null); OUT=$(realpath -m "${3}" 2>/dev/null)
KEY=${4:-key.pem}; PASS=${5:-seagull123}
uso() { echo "uso: $(basename "${0}") <tcp|tls|sctp> <dir-binarios> <dir-salida> [clave] [passwd]"; exit 1; }
case "$MODE" in
  tcp)  SUF="";      LOGSUF="" ;;
  tls)  SUF="-tls";  LOGSUF="-tls" ;;
  sctp) SUF="-sctp"; LOGSUF="" ;;
  *) uso ;;
esac
[ -d "$BIN" ] && [ -n "$OUT" ] || uso
if [ "$MODE" = sctp ]; then
  checksctp >/dev/null 2>&1 || { echo "SCTP no disponible: ejecuta sudo modprobe sctp"; exit 1; }
fi
if [ "$MODE" = tls ] && { [ ! -f "$CERTS/cert.pem" ] || [ ! -f "$CERTS/$KEY" ]; }; then
  echo "faltan certificados en $CERTS (generarlos con tests/gen-certs.sh)"; uso
fi
listening() {
  if [ "$MODE" = sctp ]; then awk -v c=6 'NR>1 && $c==3868 {f=1} END {exit !f}' /proc/net/sctp/eps 2>/dev/null
  else ss -ltn | grep -q ':3868 '; fi
}
WAIT_PORT=${WAIT_PORT:-60}; SETTLE=${SETTLE:-5}; CLIENT_MAX=${CLIENT_MAX:-180}; DRAIN=${DRAIN:-12}
if pgrep -x seagull >/dev/null || listening; then
  echo "Hay un seagull vivo o el puerto 3868 está ocupado. Ejecuta: pkill seagull"; exit 1
fi
rm -rf "$OUT"; mkdir -p "$OUT"
case "$MODE" in
  tcp)  prepare_env "$OUT" ;;
  sctp) prepare_env "$OUT"; prepare_sctp_conf "$OUT" ;;
  tls)
    prepare_env "$OUT" -tls
    c=$OUT/env/config
    sed -i '/name="number-calls" value="5"/d' "$c/conf.client-tls.xml"
    for f in "$c/conf.server-tls.xml" "$c/conf.client-tls.xml"; do
      sed -i -e "s|cert_chain_file=xxx;|cert_chain_file=$CERTS/cert.pem;|" -e "s|private_key_file=yyy;|private_key_file=$CERTS/$KEY;|" -e 's|passwd=zzz"|passwd='"$PASS"'"|' "$f"
    done ;;
esac
cd "$OUT/env/run" || exit 1
export LD_LIBRARY_PATH="$BIN"
COMMON="-dico ../config/base_cx.xml -bg -llevel ET"
getpid() { grep -oE 'PID [[][0-9]+[]]' "${1}" | grep -oE '[0-9]+'; }

SECONDS=0
"$BIN/seagull" -conf ../config/conf.server$SUF.xml -scen ../scenario/sar-saa.server.xml -log ../logs/sar-saa.server$LOGSUF.log $COMMON < /dev/null > ../logs/server.out 2>&1
SPID=$(getpid ../logs/server.out)
kill -0 "$SPID" 2>/dev/null || { echo "No se pudo obtener el PID del servidor"; cat ../logs/server.out; exit 1; }
echo "servidor PID $SPID"
OPEN=0
for i in $(seq $((WAIT_PORT * 2))); do listening && { OPEN=1; break; }; sleep 0.5; done
[ $OPEN -eq 1 ] || { echo "El puerto 3868 no se abrió en ${WAIT_PORT}s"; kill "$SPID"; exit 1; }
echo "puerto abierto a los ${SECONDS}s; margen ${SETTLE}s"; sleep "$SETTLE"
if [ "$MODE" = tls ] && [ -n "$PROBE" ]; then
  echo "--- sondeo TLS con openssl s_client:"
  echo | timeout 5 openssl s_client -connect 127.0.0.1:3868 2>&1 | grep -E 'Protocol|Cipher|error'
fi

START=$SECONDS
"$BIN/seagull" -conf ../config/conf.client$SUF.xml -scen ../scenario/sar-saa.client.xml -log ../logs/sar-saa.client$LOGSUF.log $COMMON < /dev/null > ../logs/client.out 2>&1
CPID=$(getpid ../logs/client.out)
kill -0 "$CPID" 2>/dev/null || { echo "No se pudo obtener el PID del cliente"; kill "$SPID"; exit 1; }
echo "cliente PID $CPID"
for i in $(seq $((CLIENT_MAX * 2))); do kill -0 "$CPID" 2>/dev/null || break; sleep 0.5; done
if kill -0 "$CPID" 2>/dev/null; then
  echo "El cliente NO terminó solo en ${CLIENT_MAX}s; se detiene a la fuerza"; kill "$CPID"; sleep 1; kill -9 "$CPID" 2>/dev/null
else
  echo "el cliente terminó solo tras $((SECONDS - START))s"
fi
echo "esperando ${DRAIN}s para la última fila de estadísticas"; sleep "$DRAIN"
kill "$SPID" 2>/dev/null
for i in $(seq 20); do kill -0 "$SPID" 2>/dev/null || break; sleep 0.5; done
kill -9 "$SPID" 2>/dev/null

if [ "$MODE" = tls ]; then
  echo "--- logs generados:"; ls "$OUT/env/logs"
  echo "--- errores en los logs (E/T):"; grep -hE '[|](E|T)[|]' "$OUT"/env/logs/*.log
  echo "--- SAR enviados por el cliente (esperado 100): $("$HERE/resumen.sh" "$OUT" client | cut -d';' -f10)"
  for lado in client server; do
    if diff -q "$EXPECTED/$lado.txt" <("$HERE/resumen.sh" "$OUT" $lado) >/dev/null 2>&1; then
      echo "  $lado: IGUAL a la referencia TCP"
    else
      echo "  $lado: DIFERENTE (o falta el CSV)"
    fi
  done
fi
pgrep -x seagull >/dev/null && echo "AVISO: queda algún seagull vivo" || echo "limpio"
