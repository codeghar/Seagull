// Prueba de los formateadores de BufferUtils bajo ASan/UBSan.
// Con -DPBB_OLD_API se compila contra la version anterior (debe abortar con ASan).
#include "BufferUtils.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>

static int g_fail = 0 ;
#define CHECK(c) do { if (!(c)) { std::printf("FALLO linea %d: %s", __LINE__, #c) ; std::puts("") ; g_fail++ ; } } while (0)

#define HEX_BAD(lit) { char s_ [] = lit ; size_t n_ = 99 ; \
  unsigned char* r_ = convert_hexa_char_to_bin(s_, &n_) ; \
  CHECK(r_ == nullptr) ; CHECK(n_ == 0) ; std::free(r_) ; }

// Cadenas hexadecimales de escenarios (convert_hexa_char_to_bin, modo estricto).
static void test_hexa_to_bin () {
  { char s [] = "00017f000001" ; size_t n = 99 ;
    unsigned char* r = convert_hexa_char_to_bin(s, &n) ;
    CHECK(r != nullptr) ; CHECK(n == 6) ;
    if ((r != nullptr) && (n == 6)) {
      CHECK((r[0] == 0x00) && (r[1] == 0x01) && (r[2] == 0x7f)
            && (r[3] == 0x00) && (r[4] == 0x00) && (r[5] == 0x01)) ; }
    std::free(r) ; }
  { char s [] = "DEADbeef" ; size_t n = 99 ;
    unsigned char* r = convert_hexa_char_to_bin(s, &n) ;
    CHECK(n == 4) ;
    if ((r != nullptr) && (n == 4)) {
      CHECK((r[0] == 0xde) && (r[1] == 0xad) && (r[2] == 0xbe) && (r[3] == 0xef)) ; }
    std::free(r) ; }
  { char s [] = "" ; size_t n = 99 ;
    unsigned char* r = convert_hexa_char_to_bin(s, &n) ;
    CHECK(n == 0) ; std::free(r) ; }
  HEX_BAD("zz")
  HEX_BAD("1g")
  HEX_BAD(" 1")
  HEX_BAD("-1")
  HEX_BAD("+1")
  HEX_BAD("abc")
#ifndef PBB_HEAD_RUN
  { size_t n = 99 ; unsigned char* r = convert_hexa_char_to_bin(nullptr, &n) ;
    CHECK(r == nullptr) ; CHECK(n == 0) ; }
#endif
}
#undef HEX_BAD

int main () {
  test_hexa_to_bin() ;
#ifdef PBB_HEAD_RUN
  std::printf("hexa-only: %d fallos", g_fail) ; std::puts("") ;
  return ((g_fail == 0) ? 0 : 1) ;
#endif
  unsigned char L_in [128] ;
  for (int L_i = 0 ; L_i < 128 ; L_i++) { L_in[L_i] = (unsigned char)L_i ; }

  { char b [50] ; pretty_binary_buffer(L_in, 4, b) ;
    CHECK(std::strcmp(b, "00 01 02 03 ") == 0) ; }

  { char b [50] ; pretty_binary_buffer(L_in, 16, b) ;
    CHECK(std::strlen(b) == 48) ; CHECK(std::strcmp(b + 45, "0f ") == 0) ; }

  { char b [50] ; pretty_binary_buffer(L_in, 17, b) ;
    CHECK(std::strlen(b) == 48) ; CHECK(std::strcmp(b + 45, "...") == 0) ; }

  { char b [50] ; pretty_binary_buffer(L_in, 128, b) ; CHECK(std::strlen(b) == 48) ; }

  { char b [50] ; std::memset(b, 'X', sizeof(b)) ; pretty_binary_buffer(L_in, 0, b) ;
    CHECK(b[0] == 0) ; }

  { char b [4] ; pretty_binary_buffer(L_in, 8, b) ; CHECK(std::strcmp(b, "...") == 0) ; }
  { char b [1] ; pretty_binary_buffer(L_in, 8, b) ; CHECK(b[0] == 0) ; }

  { unsigned char t [] = { 'H', 'i', 0x00, 0x7f, '~' } ; char h [50] ; char a [50] ;
    pretty_binary_printable_buffer(t, 5, h, a) ;
    CHECK(std::strcmp(h, "48 69 00 7f 7e ") == 0) ; CHECK(std::strcmp(a, "Hi..~") == 0) ; }

  { char a [50] ; pretty_ascii_printable_buffer(L_in + 32, 48, a) ; CHECK(std::strlen(a) == 48) ; }
  { char a [50] ; pretty_ascii_printable_buffer(L_in + 32, 64, a) ; CHECK(std::strlen(a) == 49) ; }

#ifndef PBB_OLD_API
  { char b [50] ;
    CHECK(pretty_binary_buffer_n(L_in, 17, b, sizeof(b)) == 15) ;
    CHECK(pretty_binary_buffer_n(L_in, 16, b, sizeof(b)) == 16) ;
    CHECK(pretty_binary_buffer_n(nullptr, 5, b, sizeof(b)) == 0) ; CHECK(b[0] == 0) ;
    CHECK(pretty_binary_buffer_n(L_in, 5, nullptr, 10) == 0) ; }
#endif

#ifndef PBB_OLD_API
  { unsigned char t [] = { 0xde, 0xad, 0x01 } ; char b [7] ;
    CHECK(convert_bin_to_hexa_char_n(t, 3, b, sizeof(b)) == 3) ; CHECK(std::strcmp(b, "dead01") == 0) ; }
  { unsigned char t [] = { 0xde, 0xad, 0x01 } ; char b [6] ;
    CHECK(convert_bin_to_hexa_char_n(t, 3, b, sizeof(b)) == 2) ; CHECK(std::strcmp(b, "dead") == 0) ; }
  { char b [4] ; std::memset(b, 'X', sizeof(b)) ;
    CHECK(convert_bin_to_hexa_char_n(L_in, 0, b, sizeof(b)) == 0) ; CHECK(b[0] == 0) ; }
  { char b [1] ; CHECK(convert_bin_to_hexa_char_n(L_in, 8, b, sizeof(b)) == 0) ; CHECK(b[0] == 0) ; }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  { unsigned char t [] = { 0x00, 0x7f, 0xff } ; char b [7] ;
    convert_bin_to_hexa_char(t, 3, b) ; CHECK(std::strcmp(b, "007fff") == 0) ; }
  { char b [4] ; std::memset(b, 'X', sizeof(b)) ;
    convert_bin_to_hexa_char(L_in, 0, b) ; CHECK(b[0] == 0) ; }
#pragma GCC diagnostic pop
#endif
  if (g_fail == 0) { std::puts("test_bufferutils: OK") ; }
  else             { std::printf("test_bufferutils: %d fallos", g_fail) ; std::puts("") ; }
  return ((g_fail == 0) ? 0 : 1) ;
}
