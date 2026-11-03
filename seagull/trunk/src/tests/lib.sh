# tests/lib.sh: rutas comunes. Uso desde otro script: . "$(dirname "$0")/lib.sh"
HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)   # .../src/tests
SRC=$(cd "$HERE/.." && pwd)                          # .../src
ENV_TEMPLATE=$SRC/exe-env/diameter-env               # ejemplo del repo (no se modifica)
EXPECTED=$HERE/expected                              # referencia gnu++98 en texto
WORK=${SEAGULL_TEST_DIR:-$HOME/seagull-test}         # salidas y certificados, fuera del repo
CERTS=$WORK/tls-test

# prepare_env <dir-salida> [sufijo, p. ej. -tls]: copia exe-env (sin modificarlo) y aplica los ajustes de prueba
prepare_env() {
  local out=${1} sfx=${2} c
  rm -rf "$out"; mkdir -p "$out"
  cp -r "$ENV_TEMPLATE" "$out/env"; mkdir -p "$out/env/logs"
  c=$out/env/config
  sed -i 's|</configuration>|  <define entity="traffic-param" name="files-no-timestamp" value="true"></define>\n</configuration>|' "$c/conf.server$sfx.xml"
  sed -i -e 's|name="call-rate" value="1"|name="call-rate" value="10"|' \
         -e 's|name="max-send" value="200"|name="max-send" value="50"|' \
         -e 's|</configuration>|  <define entity="traffic-param" name="files-no-timestamp" value="true"></define>\n  <define entity="traffic-param" name="number-calls" value="100"></define>\n</configuration>|' "$c/conf.client$sfx.xml"
}

# prepare_sctp_conf <dir-salida>: genera conf.{server,client}-sctp.xml desde las conf TCP ya ajustadas
prepare_sctp_conf() {
  local c=${1}/env/config lado
  for lado in server client; do
    sed -e 's|libtrans_ip.so|libtrans_extsctp.so|' \
        -e 's|create_cipio_instance|create_cipsctpio_instance|' \
        -e 's|delete_cipio_instance|delete_cipsctpio_instance|' \
        "$c/conf.$lado.xml" > "$c/conf.$lado-sctp.xml"
  done
}
