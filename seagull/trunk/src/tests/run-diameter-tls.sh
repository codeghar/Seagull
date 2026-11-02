#!/bin/bash
# uso: run-diameter-tls.sh <dir-binarios> <dir-salida> [clave] [passwd]   (PROBE=1 prueba s_client)   (envoltorio de run-diameter-any.sh tls)
exec "$(dirname "${0}")/run-diameter-any.sh" tls "$@"
