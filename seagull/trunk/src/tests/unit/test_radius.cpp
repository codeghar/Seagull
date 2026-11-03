#include <cstdio>
#include <cstring>
#include <openssl/evp.h>
int create_algo_MD5_radius(char*, int, char*, unsigned char*);
int main() {
  unsigned char r[16];
  char msg[] = "hola-mundo", sec[] = "secreto";
  int rc = create_algo_MD5_radius(msg, (int)strlen(msg), sec, r);
  printf("rc=%d ", rc);
  for (int i = 0; i < 16; i++) printf("%02x", r[i]);
  printf("\n");
  return 0;
}
