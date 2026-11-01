# Pruebas de transporte Diameter (SAR/SAA)

Arnés portable: todas las rutas se calculan desde la ubicación de los scripts
(`lib.sh`). Las salidas y los certificados quedan **fuera del repositorio**, en
`SEAGULL_TEST_DIR` (por defecto `~/seagull-test`).

## Requisitos

| Requisito | Para qué |
|---|---|
| `cmake`, `gcc`, `g++`, `bison`, `flex` | Compilar |
| `openssl-devel` (o el tarball de `external-lib-src/`) | TLS y crypto |
| `lksctp-tools-devel` | Solo el header `<netinet/sctp.h>` (no se enlaza `-lsctp`) |
| Módulo `sctp` cargado (`sudo modprobe sctp`) | Solo para ejecutar SCTP. No requiere reiniciar; se pierde al reiniciar |

Antes de cada sesión: `export LC_ALL=C` (los scripts buscan `warning:` en inglés).

## Estructura

| Archivo | Función |
|---|---|
| `lib.sh` | Rutas comunes, `prepare_env` y `prepare_sctp_conf` |
| `resumen.sh` | Extrae los contadores finales de un CSV de protocolo |
| `expected/` | Referencia gnu++98 en texto (cliente y servidor) |
| `unit/` | Pruebas unitarias de crypto (Radius, Digest RFC 2617) |
| `lsan.supp` | Supresiones de LeakSanitizer (solo leaks de arranque conocidos) |

El entorno de cada corrida se genera copiando `exe-env/diameter-env` (que **no se
modifica**, porque se distribuye a usuarios) y aplicando los ajustes de prueba:
`files-no-timestamp=true`, `call-rate=10`, `max-send=50`, `number-calls=100`.

## Escenarios

### TCP

```bash
cmake -S . -B build-23 -DSEAGULL_CXX_STD=23
cmake --build build-23 -j"$(nproc)"
tests/verify.sh "$PWD/build-23/bin"
```

Esperado: 100 SAR, `client: IGUAL`, `server: IGUAL`, código 0. Si la corrida no se
ejecuta (por ejemplo, ruta incorrecta), `verify.sh` termina con error y no compara.

La ruta al directorio de binarios debe ser **absoluta**.

### TLS

```bash
tests/gen-certs.sh                      # certificado autofirmado, válido 30 días
cmake -S . -B build-tls -DSEAGULL_BUILD_TLS=ON
cmake --build build-tls -j"$(nproc)"
PROBE=1 tests/run-diameter-tls.sh "$PWD/build-tls/bin" ~/seagull-test/out/tls
```

`PROBE=1` comprueba con `openssl s_client` qué protocolo se negocia (TLS 1.3).
Plugin: `libtrans_iptls.so`, `create_ciptlsio_instance`.

### SCTP nativo

```bash
sudo modprobe sctp && checksctp          # debe responder "SCTP supported"
cmake -S . -B build-sctp -DSEAGULL_BUILD_SCTP=ON
cmake --build build-sctp -j1
tests/run-diameter-sctp.sh "$PWD/build-sctp/bin" ~/seagull-test/out/sctp
```

Plugin: `libtrans_extsctp.so`, `create_cipsctpio_instance`. Usa SCTP del kernel
(sin `sctplib` ni `socketapi`). El puerto se detecta en `/proc/net/sctp/eps`,
porque `ss -lnS` no muestra sockets SCTP si `sctp_diag` no está cargado.

En RHEL/AlmaLinux el módulo está en blacklist por defecto. La habilitación
permanente en producción debe acordarse con el equipo de seguridad.

### Carga

```bash
MODE=tcp RATE=200 CALLS=2000 tests/run-diameter-load.sh "$PWD/build-release/bin" ~/seagull-test/out/load-tcp
MODE=tls RATE=200 CALLS=2000 tests/run-diameter-load.sh "$PWD/build-release/bin" ~/seagull-test/out/load-tls
```

Usar un build **Release**: los builds Debug no sostienen 200 llamadas/s en una VM
pequeña y las llamadas expiran. Variables opcionales: `SIM`, `BIG`, `KEY`, `PASS`.

### Validación completa

```bash
tests/validate-all.sh            # STD_LOOP=1 compila además con C++98..23
```

Recompila desde cero `build-werror`, `build-werror-ssl`, `build-asan` y
`build-release`, y ejecuta crypto, TCP y TLS con ASan, carga y SCTP (si el módulo
no está cargado, esa etapa queda como AVISO). Logs en
`SEAGULL_TEST_DIR/validation/<fecha>`. Tarda varios minutos: conviene `nohup`.

## Notas conocidas

- **Contador `SAA received(C)`**: Seagull nunca lo incrementa en el cliente. SAR y
  SAA comparten el código de comando 301 y las respuestas se cuentan como
  `SAR received`. Para validar el cliente usar `TrafficSuccessfulCall(C)` y
  `FailedCall(C)` de `client-stat.csv`.
- **Leaks de arranque**: el parser XML, la línea de comandos y las pantallas de
  display reservan memoria que no se libera hasta salir. No crecen con el tráfico;
  están en `lsan.supp`. Cualquier otro leak se sigue reportando.
- **Certificados**: no se versionan (`*.pem` y `*.key` están en `.gitignore`).
  Se regeneran con `tests/gen-certs.sh` cuando caducan.
- **Buffers de SCTP**: con `buffer=` pequeño (por ejemplo 4096), el SCTP del kernel no
  sostiene la carga y las llamadas vencen. Usar el valor por defecto.
- **Un cliente por servidor**: el `<init>` del escenario servidor se ejecuta una vez por
  proceso; para lanzar otro cliente hay que reiniciar el servidor.

## Solución de problemas

| Síntoma | Revisión |
|---|---|
| `Hay un seagull vivo o el puerto 3868 está ocupado` | `pkill -x seagull` |
| `SCTP no disponible` | `sudo modprobe sctp` |
| `El puerto 3868 no se abrió` (SCTP) | `cat /proc/net/sctp/eps` con el servidor corriendo |
| `servidor no arranco` (TLS) | Contraseña o certificados: `tests/gen-certs.sh` |
| Falla la carga con un build Debug | Usar `build-release` o bajar `RATE` |
| El cliente tarda 15-25 s antes de enviar tráfico (solo con `type=udp` o con `source=`) | El *hostname* no se resuelve localmente y el DNS pierde consultas: agregarlo a `/etc/hosts` (comprobar con `time getent ahostsv4 "$(hostname)"`, debe ser instantáneo). Con `type=tcp` sin `source=` ya no se resuelve el *hostname* |
