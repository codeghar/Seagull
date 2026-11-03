#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int createAuthHeaderMD5(char*,char*,char*,char*,char*,char*,char*,char*);
int main(int argc, char **argv) {
  char res[2049];
  char a1[] = "Digest realm=\"testrealm@host.com\", nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\"";
  char a2[] = "Digest realm=\"testrealm@host.com\", qop=\"auth\", nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"";
  char *big;
  int rc;
  srand(1);
  if (argc > 1) {                      /* modo "long": realm de 3000 caracteres */
    big = calloc(1, 3100);
    strcpy(big, "Digest nonce=\"n\", realm=\"");
    memset(big + strlen(big), 'A', 3000);
    strcpy(big + strlen(big), "\"");
    rc = createAuthHeaderMD5("Mufasa","pw","GET","/x","",big,"MD5",res);
    printf("long: rc=%d strlen=%lu\n", rc, (unsigned long)strlen(res));
    return 0;
  }
  rc = createAuthHeaderMD5("Mufasa","Circle Of Life","GET","/dir/index.html","",a1,"MD5",res);
  printf("sin qop: rc=%d\n%s\n\n", rc, res);
  rc = createAuthHeaderMD5("Mufasa","Circle Of Life","GET","/dir/index.html","",a2,"MD5",res);
  printf("con qop: rc=%d\n%s\n", rc, res);
  return 0;
}
