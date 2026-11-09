#!/bin/bash
# uso: resumen.sh <dir-salida> [client|server]
awk -F';' 'END{for(i=7;i<NF;i+=2) printf "%s;", $i; print ""}' "$1/env/logs/${2:-client}-protocol-stat.diameter-v1.csv"
