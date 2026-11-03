# Diameter Gx (3GPP TS 29.212)

Soporte Gx agregado durante la modernizacion. No existe en el RPM original
seagull-diameter-protocol 1.8.2 de HP.

## Archivos

| Archivo | Descripcion |
|---|---|
| `config/base_gx.xml` | Diccionario Gx (Auth-Application-Id 16777238, Vendor 3GPP 10415) |
| `scenario/gx-ccr-cca.server.xml` | PCRF simulado: CEA, CCA-I con politica, CCA-T, DWA |
| `scenario/gx-ccr-cca.client.xml` | PCEF simulado: CER, CCR-I, CCR-T, DWA |
| `scenario/gx-session.server.xml` | PCRF simulado: CEA, CCA-I, RAR, CCA-U, CCA-T, DWA |
| `scenario/gx-session.client.xml` | PCEF simulado: CER, CCR-I, RAA, CCR-U, CCR-T, DWA |
| `run/start_server_gx.ksh` / `run/start_client_gx.ksh` | Arranque interactivo |

## Flujo (gx-ccr-cca)

```
PCEF (cliente)                 PCRF (servidor)
   | CER ------------------------> |
   | <------------------------ CEA |
   | CCR-I (Request-Type 1) -----> |
   | <------ CCA-I + Charging-Rule |
   | CCR-T (Request-Type 3) -----> |   Destination-Host = Origin-Host del CCA-I
   | <---------------------- CCA-T |
   | DWR <-> DWA  (<default>)      |
```

## Sesion completa (gx-session)

Agrega CCR-U y un RAR/RAA iniciado por el PCRF a mitad de la llamada. El RAR viaja
por la misma conexion; el PCEF lo asocia a la llamada abierta por `Session-Id`
(`<command session-id="Session-Id" ...>` en el diccionario).

```
PCEF (cliente)                 PCRF (servidor)
   | CER ------------------------> |
   | <------------------------ CEA |
   | CCR-I (Number 0) -----------> |   el PCRF guarda Origin-Host/Realm del PCEF
   | <-- CCA-I + Install + Trigger |   Event-Trigger 1 QOS_CHANGE
   | <----- RAR (Remove + Install) |   HbH/EtE propios del PCRF (contadores 5000/6000)
   | RAA 2001 -------------------> |   mismos HbH/EtE del RAR
   | CCR-U (Number 1) -----------> |   Event-Trigger 1
   | <---------------------- CCA-U |
   | CCR-T (Number 2) -----------> |
   | <---------------------- CCA-T |
```

Validaciones (`check-value`, behaviour=error):

- Servidor: `CC-Request-Type` 1, 2 y 3 en cada CCR (detecta mensajes fuera de orden)
  y `Result-Code` 2001 en el RAA.
- Cliente: `Re-Auth-Request-Type` 0 en el RAR y `Result-Code` 2001 en CCA-I, CCA-U y CCA-T.

Ejecucion automatizada desde `tests/` (ver `tests/README.md`):

```
DICO=base_gx.xml SCEN=gx-session ./run-diameter-any.sh tcp <dir-binarios> <dir-salida>
```

`tcp` se puede cambiar por `tls` o `sctp`. El script termina con `RESULTADO: OK` (codigo 0),
o con `RESULTADO: FALLO` (codigo 2) si hay lineas `|E|` en los logs o queda un seagull vivo.
`OutgoingCall(C)` = 101 es normal: el `<init>` (CER/CEA) cuenta como una llamada mas.

## AVPs principales

| AVP | Codigo | Vendor | Mensaje | Valor de prueba |
|---|---|---|---|---|
| Session-Id | 263 | - | CCR/CCA | `client.gx.example.com;1;N` |
| Auth-Application-Id | 258 | - | CER/CCR/CCA | 16777238 (Gx) |
| Destination-Host | 293 | - | CCR-U/T, RAR | CCR: Origin-Host del CCA-I; RAR: Origin-Host del CCR-I |
| CC-Request-Type | 416 | - | CCR/CCA | 1 INITIAL, 2 UPDATE, 3 TERMINATION |
| CC-Request-Number | 415 | - | CCR/CCA | 0-1 (gx-ccr-cca), 0-2 (gx-session) |
| Termination-Cause | 295 | - | CCR-T | 1 DIAMETER_LOGOUT |
| Re-Auth-Request-Type | 285 | - | RAR | 0 AUTHORIZE_ONLY |
| Subscription-Id (Type 450 / Data 444) | 443 | - | CCR-I | E.164 y IMSI |
| Framed-IP-Address | 8 | - | CCR-I | 10.0.0.5 |
| Called-Station-Id | 30 | - | CCR-I | `internet.test` |
| Bearer-Operation | 1021 | 10415 | CCR-I | 1 ESTABLISHMENT |
| IP-CAN-Type | 1027 | 10415 | CCR-I | 5 3GPP-EPS |
| RAT-Type | 1032 | 10415 | CCR-I | 1004 EUTRAN |
| AN-GW-Address | 1050 | 10415 | CCR-I | IP del cliente |
| QoS-Information (APN-AMBR UL 1041 / DL 1040) | 1016 | 10415 | CCR-I/CCA-I | 50-100 Mbps |
| Default-EPS-Bearer-QoS (QCI 1028, ARP 1034) | 1049 | 10415 | CCR-I/CCA-I | QCI 9, Priority 10 |
| Event-Trigger | 1006 | 10415 | CCA-I/CCR-U | 1 QOS_CHANGE |
| Charging-Rule-Install (Rule-Name 1005) | 1001 | 10415 | CCA-I/RAR | `rule_default_test`, `rule_rar_test` |
| Charging-Rule-Remove (Rule-Name 1005) | 1002 | 10415 | RAR | `rule_default_test` |
| Supported-Features (629 / 630) | 628 | 10415 | CCR-I/CCA-I | List-ID 1, List 2 |
| 3GPP-SGSN-MCC-MNC | 18 | 10415 | CCR-I | 00101 |

## Prueba en loopback

Los archivos usan 127.0.0.1:3868 por defecto. Desde `run/`:

```
./start_server_gx.ksh      # terminal 1
./start_client_gx.ksh      # terminal 2
```

En ejecuciones automatizadas (`</dev/null`) agregue `-bg`; sin terminal,
Seagull aborta con `Fatal: Keyboard saved configuration failure`.

## Prueba entre VMs

1. `config/conf.server.xml`: `open-args="mode=server;source=<IP-servidor>:<puerto>"`
2. `config/conf.client.xml`: `open-args="mode=client;dest=<IP-servidor>:<puerto>"`
3. AVPs de tipo Address en los escenarios (`Host-IP-Address`, `AN-GW-Address`).
   Formato: `0x0001` (AddressType IPv4) + IPv4 en hexadecimal:
   ```
   printf '0x0001%02x%02x%02x%02x\n' 192 168 63 131    # -> 0x0001c0a83f83
   ```
   `Framed-IP-Address` es OctetString: solo 4 bytes, sin el prefijo `0x0001`.
4. Abrir el puerto en el firewall del servidor:
   ```
   sudo firewall-cmd --add-port=<puerto>/tcp
   ```

## Validacion

- Usar `TrafficSuccessfulCall(C)` del CSV de estadisticas.
- El cliente valida `Result-Code = 2001` en CCA-I y CCA-T (`check-value`,
  behaviour=error): un CCA con 5xxx cuenta como `FailedCall` y `UnexpectedMsg`.
- Evidencia a nivel de paquete (si el puerto no es 3868, agregar `-d tcp.port==<puerto>,diameter`):

```
tshark -r gx.pcap -Y diameter -T fields -e diameter.cmd.code -e diameter.flags.request -e diameter.CC-Request-Type -e diameter.Result-Code | sort | uniq -c
```

| Prueba | Resultado |
|---|---|
| Loopback, build C++23 | 20/20, Result-Code 2001 en las 40 respuestas |
| Loopback, Destination-Host en CCR-T | `pcrf.example.com` en los 20 CCR-T |
| Loopback, negativa (CCA-T = 5002) | Cliente: 0 exitosas, 20 FailedCall |
| Loopback, negativa (CCA-I = 5002) | Cliente: 0 exitosas, 20 FailedCall |
| Loopback, DWR hacia el servidor | 20 DWR / 20 DWA 2001 (`DefaultSuccessfulCall` = 20) |
| Entre VMs AlmaLinux 10.2, mismo build C++23 | 100/100, 0 fallos; 100 CCR-I/CCA-I y 100 CCR-T/CCA-T con 2001; Destination-Host en los 100 CCR-T |
| Loopback, TLS 1.2 y TLS 1.3 (`OPENSSL_CONF`) | 20/20 en cada version; negativa TLS 1.3 (CCA-T = 5002): 0 exitosas |
| Loopback, TLS 1.2 y 1.3 con `netem` 20 ms | 20/20 (antes del arreglo `1b57a09`: 0/0, `SSL_ERROR_WANT_WRITE`) |
| Entre VMs, TLS 1.2 y 1.3 (OpenSSL 3.5.8 / 3.5.5) | 100/100 en 6 corridas; 0 mensajes Diameter en claro |
| Loopback, SCTP nativo | 20/20; negativa 0 exitosas; Destination-Host en los 20 CCR-T |
| Entre VMs, SCTP nativo | 100/100; negativa 0 exitosas, 100 FailedCall; Destination-Host en los 100 CCR-T |
| Loopback, `gx-ccr-cca` y `gx-session` en TCP, TLS y SCTP (`run-diameter-any.sh`) | 100/100 en los 6 casos, 0 FailedCall, 0 lineas de error en los logs |
| Loopback, `gx-session`: RAR iniciado por el PCRF | 100 RAR recibidos y 100 RAA enviados por el cliente, correlacionados por Session-Id |

## Librerias

Los binarios generados con CMake usan `RPATH=$ORIGIN` y cargan las `.so` de su
propio directorio. `LD_LIBRARY_PATH` solo es necesario en instalaciones con
`make install`; si las librerias estan en otra ruta:

```
SEAGULL_LIB_DIR=<ruta> ./start_server_gx.ksh
```

## Transportes TLS y SCTP

Los mismos `base_gx.xml` y `gx-ccr-cca.*.xml` funcionan sobre TCP, TLS y SCTP;
solo cambia el archivo `conf`.

### TLS

- Usar `conf.server-tls.xml` / `conf.client-tls.xml` y reemplazar en `init-args`
  `cert_chain_file`, `private_key_file` y `passwd`.
- Ambos lados deben usar el mismo certificado (cada lado lo carga tambien como CA).
- `method=SSLv23` negocia la version mas alta comun. Para fijarla, usar un archivo
  de configuracion de OpenSSL:

```
[system_default_sect]
MinProtocol = TLSv1.2
MaxProtocol = TLSv1.2
```

  y lanzar con `OPENSSL_CONF=<archivo> seagull ...`.
- Wireshark asocia el puerto 3868 a Diameter sobre TCP: usar
  Decode As TCP port 3868 (o el que corresponda) como TLS.
- Solo TLS 1.2 con intercambio RSA (por ejemplo `CipherString = AES256-GCM-SHA384`)
  se puede descifrar con la clave privada. TLS 1.3 usa claves efimeras.

### SCTP

- Compilar con `-DSEAGULL_BUILD_SCTP=ON` (genera `libtrans_extsctp.so`; usa el
  SCTP del kernel, sin depender de lksctp-tools en ejecucion).
- Cargar el modulo en ambas maquinas: `sudo modprobe sctp`.
- Usar `config/conf.server-sctp.xml` y `config/conf.client-sctp.xml`: son iguales a los
  de TCP salvo el plugin (`libtrans_extsctp.so`, `create_cipsctpio_instance` /
  `delete_cipsctpio_instance`).
- No hay scripts `start_*_sctp.ksh`: tomar como referencia `run/start_server_gx.ksh` y
  `run/start_client_gx.ksh` y cambiar `-conf` por el archivo `-sctp.xml`.
- Abrir el puerto como SCTP: `sudo firewall-cmd --add-port=<puerto>/sctp`.
- Para ver si el servidor escucha: `cat /proc/net/sctp/eps`
  (`ss -S` necesita el modulo `sctp_diag`).
- Si el puerto no es 3868, en Wireshark/tshark: `-d sctp.port==<puerto>,diameter`.
- Usar el tamano de buffer por defecto (65535) o mayor. Con `buffer=4096` en `open-args`,
  SCTP queda limitado por el kernel (ventana de ~4 KB, SACK diferido, `rto_min` 1 s;
  unos 25 mensajes/s) y no sostiene 50 llamadas/s.

## Comportamiento del decodificador de Seagull

Reglas observadas durante las pruebas; tenerlas en cuenta al editar diccionarios:

- Al recibir, cada AVP se busca solo por `avp-code` y gana la primera definicion del
  archivo; el Vendor-Id no se usa. Dos AVPs con el mismo codigo colisionan aunque sean de
  distinto vendor (ejemplo: Allot-Usage-Monitoring 1002/21274 ocultaba a
  Charging-Rule-Remove 1002/10415 y su contenido se perdia).
- Al enviar, cada AVP se busca por nombre: flags, Vendor-Id y tipo salen de la definicion
  del diccionario. Un AVP mal definido se recibe bien, pero se envia mal.
- Los comandos recibidos se identifican solo por `cmd-code`: las estadisticas de protocolo
  cuentan los CCA como `CCR received` y los CEA como `CER received`. Usar
  `TrafficSuccessfulCall(C)` como criterio.
- Los flags y el Vendor-Id que muestra el log (`-llevel A`) vienen del diccionario, no del
  mensaje. Para ver el contenido real del cable, usar Wireshark/tshark.
- Un entero recibido admite hasta 8 bytes. `Current Size: N is greater than expecting one: 8`
  indica que un AVP Grouped u OctetString se decodifico con una definicion entera.
- 0 lineas `|E|` no garantiza valores correctos: validar los AVPs clave con `check-value`.

## Mantenimiento de base_gx.xml

- Bit P (Proxyable): CCR y RAR con flags 192, CCA y RAA con 64, como en una traza
  CPS <-> PCEF de laboratorio. CER/CEA, DWR/DWA y DPR/DPA van sin bit P (RFC 6733).
- Duplicados: las definiciones secundarias estan comentadas con `<!-- DUP: ... -->`.
  No reintroducir codigos AVP ni nombres de comando duplicados. Comprobacion (salida vacia):

  ```
  sed '/<!-- DUP/,/-->/d' config/base_gx.xml | grep -o 'avp-code" value="[0-9]*"' | sort | uniq -d
  ```

- Vendor por rango de codigo:

  | Rango | Vendor esperado |
  |---|---|
  | 1-30 | VSAs de 3GPP (TS 29.061), 10415 |
  | 31-499 | Base IETF (RFC 6733/4006), sin vendor, flags 64 |
  | 500 en adelante | 3GPP, 10415 |

- Validacion XML: el encabezado de licencia esta antes de `<?xml ...?>`, por eso `xmllint`
  falla sobre el archivo completo. Validar desde la declaracion:

  ```
  sed '1,/^<?xml/{/^<?xml/!d}' config/base_gx.xml | xmllint --noout -
  ```

- Despues de cualquier cambio, correr la regresion Gx de `tests/README.md` (6 casos).

## Limitaciones conocidas

- El `<default>` DWR/DWA del cliente no se ha ejercitado (requiere un peer que envie DWR, p. ej. CPS).
- El temporizador del cliente mide solo CCR-I/CCA-I (latencia de establecimiento).
- TLS: sin `crl_file` en `init-args`, Seagull cifra pero no verifica el certificado del peer (SSL_VERIFY_NONE). Con `crl_file` exige certificado y lo valida contra `cert_chain_file`.
- Un servidor Seagull atiende un solo cliente por ejecucion: el bloque `<init>` (CER/CEA)
  se ejecuta una vez por proceso, y el CER de un segundo cliente se trata como inesperado.
  Hay que reiniciar el servidor para cada cliente (aplica a TCP, TLS y SCTP).
- AVPs Cx heredados con numeracion antigua (2, 3, 15, 24, 25, 26): coinciden con VSAs de
  TS 29.061. No se usan en Gx ni se observaron en trafico real; comentarlos si aparece una
  colision.
- AVPs de reglas dinamicas VoLTE no definidos: Service-Identifier (439),
  AF-Charging-Identifier (505), Flow-Number (509), Flows (510), Media-Component-Number (518),
  Revalidation-Time (1042) y Flow-Direction (1080). Agregarlos antes de usar Seagull como
  PCEF frente a un PCRF con sesiones Rx.
