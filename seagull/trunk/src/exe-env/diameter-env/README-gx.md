# Diameter Gx (3GPP TS 29.212)

Soporte Gx agregado durante la modernizacion. No existe en el RPM original
seagull-diameter-protocol 1.8.2 de HP.

## Archivos

| Archivo | Descripcion |
|---|---|
| `config/base_gx.xml` | Diccionario Gx (Auth-Application-Id 16777238, Vendor 3GPP 10415) |
| `scenario/gx-ccr-cca.server.xml` | PCRF simulado: CEA, CCA-I con politica, CCA-T, DWA |
| `scenario/gx-ccr-cca.client.xml` | PCEF simulado: CER, CCR-I, CCR-T, DWA |
| `run/start_server_gx.ksh` / `run/start_client_gx.ksh` | Arranque interactivo |

## Flujo

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

## AVPs principales

| AVP | Codigo | Vendor | Mensaje | Valor de prueba |
|---|---|---|---|---|
| Session-Id | 263 | - | CCR/CCA | `client.gx.example.com;1;N` |
| Auth-Application-Id | 258 | - | CER/CCR/CCA | 16777238 (Gx) |
| Destination-Host | 293 | - | CCR-T | Origin-Host del CCA-I |
| CC-Request-Type | 416 | - | CCR/CCA | 1 INITIAL, 3 TERMINATION |
| CC-Request-Number | 415 | - | CCR/CCA | 0, 1 |
| Termination-Cause | 295 | - | CCR-T | 1 DIAMETER_LOGOUT |
| Subscription-Id (Type 450 / Data 444) | 443 | - | CCR-I | E.164 y IMSI |
| Framed-IP-Address | 8 | - | CCR-I | 10.0.0.5 |
| Called-Station-Id | 30 | - | CCR-I | `internet.test` |
| Bearer-Operation | 1021 | 10415 | CCR-I | 1 ESTABLISHMENT |
| IP-CAN-Type | 1027 | 10415 | CCR-I | 5 3GPP-EPS |
| RAT-Type | 1032 | 10415 | CCR-I | 1004 EUTRAN |
| AN-GW-Address | 1050 | 10415 | CCR-I | IP del cliente |
| QoS-Information (APN-AMBR UL 1041 / DL 1040) | 1016 | 10415 | CCR-I/CCA-I | 50-100 Mbps |
| Default-EPS-Bearer-QoS (QCI 1028, ARP 1034) | 1049 | 10415 | CCR-I/CCA-I | QCI 9, Priority 10 |
| Event-Trigger | 1006 | 10415 | CCA-I | 1 QOS_CHANGE |
| Charging-Rule-Install (Rule-Name 1005) | 1001 | 10415 | CCA-I | `rule_default_test` |
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

## Limitaciones conocidas

- El `<default>` DWR/DWA del cliente no se ha ejercitado (requiere un peer que envie DWR, p. ej. CPS).
- No hay CCR-U ni RAR/RAA.
- El temporizador del cliente mide solo CCR-I/CCA-I (latencia de establecimiento).
- TLS: sin `crl_file` en `init-args`, Seagull cifra pero no verifica el certificado del peer (SSL_VERIFY_NONE). Con `crl_file` exige certificado y lo valida contra `cert_chain_file`.
- Un servidor Seagull atiende un solo cliente por ejecucion: el bloque `<init>` (CER/CEA)
  se ejecuta una vez por proceso, y el CER de un segundo cliente se trata como inesperado.
  Hay que reiniciar el servidor para cada cliente (aplica a TCP, TLS y SCTP).
