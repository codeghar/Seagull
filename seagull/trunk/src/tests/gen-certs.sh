#!/bin/bash
# uso: gen-certs.sh [passwd]   genera certificado autofirmado de laboratorio en $CERTS
. "$(dirname "${0}")/lib.sh"
PASS=${1:-seagull123}; DAYS=${DAYS:-30}
mkdir -p "$CERTS" && cd "$CERTS" || exit 1
openssl req -x509 -newkey rsa:2048 -sha256 -days "$DAYS" -nodes \
  -keyout key-plain.pem -out cert.pem -subj "/CN=127.0.0.1" -addext "subjectAltName=IP:127.0.0.1" || exit 1
openssl pkey -in key-plain.pem -aes256 -passout pass:"$PASS" -out key.pem || exit 1
chmod 600 key.pem key-plain.pem
echo "certificados en $CERTS"
openssl x509 -in cert.pem -noout -enddate
