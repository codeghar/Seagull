#!/bin/bash
# uso: run-diameter-sctp.sh <dir-binarios> <dir-salida>   (envoltorio de run-diameter-any.sh sctp)
exec "$(dirname "${0}")/run-diameter-any.sh" sctp "$@"
