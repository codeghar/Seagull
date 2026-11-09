#!/bin/bash
# uso: MODE=tcp|tls RATE=500 CALLS=20000 run-diameter-load.sh <dir-binarios> <dir-salida>
. "$(dirname "${0}")/lib.sh"
BIN=$(realpath "$1"); OUT=$(realpath -m "$2")
MODE=${MODE:-tls}; RATE=${RATE:-500}; CALLS=${CALLS:-20000}
T=$CERTS; KEY=${KEY:-key.pem}; PASS=${PASS:-seagull123}
[ -d "$BIN" ] && [ -n "$OUT" ] || { echo "uso: MODE=tcp|tls RATE=n CALLS=n $0 <dir-binarios> <dir-salida>"; exit 1; }
case "$MODE" in tcp) SUF="";; tls) SUF="-tls";; *) echo "MODE debe ser tcp o tls"; exit 1;; esac
if pgrep -x seagull >/dev/null || ss -ltn | grep -q ':3868 '; then
  echo "Hay un seagull vivo o el puerto 3868 esta ocupado. Ejecuta: pkill seagull"; exit 1
fi
BIG=${BIG:-$((CALLS * 20))}; SIM=${SIM:-5000}
CLIENT_MAX=$((CALLS / RATE * 3 + 60)); DRAIN=${DRAIN:-12}

rm -rf "$OUT"; mkdir -p "$OUT"
cp -r "$ENV_TEMPLATE" "$OUT/env"; mkdir -p "$OUT/env/logs"
cd "$OUT/env/config" || exit 1
for f in conf.server$SUF.xml conf.client$SUF.xml; do
  sed -i -e '/name="max-send"/d' -e '/name="max-receive"/d' -e '/name="max-simultaneous-calls"/d' \
         -e '/name="number-calls"/d' -e '/name="call-rate"/d' -e '/files-no-timestamp/d' "$f"
  EXTRA="  <define entity=\"traffic-param\" name=\"max-send\" value=\"$BIG\"></define>\n  <define entity=\"traffic-param\" name=\"max-receive\" value=\"$BIG\"></define>\n  <define entity=\"traffic-param\" name=\"max-simultaneous-calls\" value=\"$SIM\"></define>\n  <define entity=\"traffic-param\" name=\"files-no-timestamp\" value=\"true\"></define>"
  [ "$f" = "conf.client$SUF.xml" ] && EXTRA="$EXTRA\n  <define entity=\"traffic-param\" name=\"call-rate\" value=\"$RATE\"></define>\n  <define entity=\"traffic-param\" name=\"number-calls\" value=\"$CALLS\"></define>"
  sed -i "s|</configuration>|$EXTRA\n</configuration>|" "$f"
  if [ "$MODE" = tls ]; then
    sed -i -e "s|cert_chain_file=xxx;|cert_chain_file=$T/cert.pem;|" \
           -e "s|private_key_file=yyy;|private_key_file=$T/$KEY;|" -e "s|passwd=zzz\"|passwd=$PASS\"|" "$f"
  fi
done
echo "modo=$MODE rate=$RATE calls=$CALLS"
grep -h 'call-rate\|number-calls\|max-send' conf.client$SUF.xml

cd "$OUT/env/run" || exit 1
export LD_LIBRARY_PATH="$BIN"
COMMON="-dico ../config/base_cx.xml -bg -llevel ET"
getpid() { sed -n 's/.*PID \[\([0-9]*\)\].*/\1/p' "$1"; }
"$BIN/seagull" -conf ../config/conf.server$SUF.xml -scen ../scenario/sar-saa.server.xml \
  -log ../logs/sar-saa.server.log $COMMON < /dev/null > ../logs/server.out 2>&1
SPID=$(getpid ../logs/server.out); kill -0 "$SPID" 2>/dev/null || { echo "servidor no arranco"; cat ../logs/server.out; exit 1; }
for i in $(seq 120); do ss -ltn | grep -q ':3868 ' && break; sleep 0.5; done; sleep 5
SECONDS=0
"$BIN/seagull" -conf ../config/conf.client$SUF.xml -scen ../scenario/sar-saa.client.xml \
  -log ../logs/sar-saa.client.log $COMMON < /dev/null > ../logs/client.out 2>&1
CPID=$(getpid ../logs/client.out); kill -0 "$CPID" 2>/dev/null || { echo "cliente no arranco"; cat ../logs/client.out; kill "$SPID"; exit 1; }
for i in $(seq $((CLIENT_MAX * 2))); do kill -0 "$CPID" 2>/dev/null || break; sleep 0.5; done
DUR=$SECONDS
if kill -0 "$CPID" 2>/dev/null; then echo "El cliente NO termino solo en ${CLIENT_MAX}s"; kill "$CPID"; sleep 1; kill -9 "$CPID" 2>/dev/null; DONE=no; else DONE=si; fi
sleep "$DRAIN"
kill -0 "$SPID" 2>/dev/null && SALIVE=si || SALIVE=NO
kill "$SPID" 2>/dev/null; for i in $(seq 20); do kill -0 "$SPID" 2>/dev/null || break; sleep 0.5; done; kill -9 "$SPID" 2>/dev/null

csvval() { awk -F';' -v n="$2" 'NR==1{for(i=1;i<=NF;i++)if($i==n)c=i} {l=$0} END{split(l,a,";");print a[c]}' "$1"; }
CS=$OUT/env/logs/client-protocol-stat.diameter-v1.csv; SS=$OUT/env/logs/server-protocol-stat.diameter-v1.csv
ST=$OUT/env/logs/client-stat.csv
A=$(csvval "$CS" 'SAR sent(C)'); B=$(csvval "$ST" 'TrafficSuccessfulCall(C)'); F=$(csvval "$ST" 'FailedCall(C)')
C=$(csvval "$SS" 'SAR received(C)'); D=$(csvval "$SS" 'SAA sent(C)')
echo "--- resultado (esperado $CALLS en los cuatro; fallidas 0)"
echo "cliente: SAR enviados=$A  llamadas exitosas=$B  fallidas=$F"
echo "servidor: SAR recibidos=$C  SAA enviados=$D"
echo "cliente termino solo: $DONE; servidor vivo al final: $SALIVE; duracion cliente: ${DUR}s"
awk -F';' 'NR==1{for(i=1;i<=NF;i++){if($i=="Rate(P)")p=i; if($i=="ElapsedTime(C)")e=i}; next} $e!=prev && $p>0 {v[++n]=$p; if($p>m)m=$p} {prev=$e} END{for(i=1;i<=n;i++) if(v[i]>=m/2){s+=v[i]; k++}; if(k) print sprintf("tasa efectiva: media %.0f/s, maxima %.0f/s (%d periodos con trafico)", s/k, m, k); else print "tasa efectiva: sin periodos con trafico"}' "$ST"
echo "--- errores E/T distintos del cierre normal:"
grep -hE '\|(E|T)\|' "$OUT"/env/logs/*.log | grep -v 'closed'
echo "--- informes de sanitizers:"
grep -l -E 'AddressSanitizer|LeakSanitizer|runtime error' "$OUT"/env/logs/*.out "$OUT"/env/logs/*.log 2>/dev/null || echo "ninguno"
if [ "$A" = "$CALLS" ] && [ "$B" = "$CALLS" ] && [ "$F" = "0" ] && [ "$C" = "$CALLS" ] && [ "$D" = "$CALLS" ]; then echo "RESULTADO: OK"; else echo "RESULTADO: FALLO (contadores distintos de $CALLS)"; fi
pgrep -x seagull >/dev/null && echo "AVISO: queda algun seagull vivo" || echo "limpio"
