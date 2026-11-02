#!/bin/bash
# uso: run-diameter.sh <dir-binarios> <dir-salida>   (envoltorio de run-diameter-any.sh tcp)
exec "$(dirname "${0}")/run-diameter-any.sh" tcp "$@"
