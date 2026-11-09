cmake_minimum_required(VERSION 3.22)
project(seagull VERSION 1.8.3 LANGUAGES C CXX)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# ---- Opciones ----
set(SEAGULL_CXX_STD "23" CACHE STRING "Estándar de C++: 98 11 14 17 20 23")
set_property(CACHE SEAGULL_CXX_STD PROPERTY STRINGS 98 11 14 17 20 23)
set(_seagull_allowed_std 98 11 14 17 20 23)
if(NOT SEAGULL_CXX_STD IN_LIST _seagull_allowed_std)
  message(FATAL_ERROR "SEAGULL_CXX_STD='${SEAGULL_CXX_STD}' no es válido. Valores permitidos: ${_seagull_allowed_std}")
endif()
option(SEAGULL_WERROR       "Tratar advertencias como errores" OFF)
option(SEAGULL_SANITIZE     "Activar ASan y UBSan"             OFF)
option(SEAGULL_BUILD_CRYPTO "Compilar lib_crypto.so"           OFF)
option(SEAGULL_BUILD_TLS    "Compilar libtrans_iptls.so (OpenSSL)" OFF)
option(SEAGULL_BUILD_SCTP   "Compilar libtrans_extsctp.so (SCTP nativo del kernel)" OFF)

# ---- Estándares ----
set(CMAKE_CXX_STANDARD ${SEAGULL_CXX_STD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)          # gnu++XX, como el original
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_EXTENSIONS ON)            # gnu90
set(CMAKE_POSITION_INDEPENDENT_CODE ON)   # -fPIC
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

get_property(_multi GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _multi AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Baseline CACHE STRING "Baseline (equivalente a build.conf), Debug, Release o RelWithDebInfo" FORCE)
endif()
set(CMAKE_C_FLAGS_BASELINE "")
set(CMAKE_CXX_FLAGS_BASELINE "")

# ---- Flags comunes (de build.conf, sección LINUX) ----
add_compile_definitions(VERSION="${PROJECT_VERSION}" _GNU_SOURCE _REENTRANT)
add_compile_definitions($<$<CONFIG:Debug>:DEBUG_MODE>
                        $<$<CONFIG:Debug>:LOG_DEBUG_NEW>)
add_compile_options(-Wall)
if(SEAGULL_WERROR)
  add_compile_options(-Werror)
endif()
if(SEAGULL_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---- Módulos: cada carpeta es un directorio de includes ----
set(SEAGULL_ALL_MODULES
  common generator-common generator-core generator-model protocol-frame
  protocol-data protocol-binary protocol-binary-body-with-separator
  protocol-tlv protocol-binary-body-not-interpreted protocol-external
  protocol-text protocol-stats scenario-stats data-log transport-frame
  generator-traffic generator-ext-AgnerLib generator-scenario external-data
  xml-parser generator-stats parser-frame filter-frame command-actions
  generator-control library-trans-ip library-parser-xml library-parser-h248
  library-parser-msrp library-general-methods library-crypto library-trans-tls)
foreach(m ${SEAGULL_ALL_MODULES})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${m})
endforeach()

# Reúne los .cpp y .c de una lista de módulos
function(seagull_collect out)
  set(files)
  foreach(m ${ARGN})
    file(GLOB f CONFIGURE_DEPENDS
         ${CMAKE_CURRENT_SOURCE_DIR}/${m}/*.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/${m}/*.c)
    list(APPEND files ${f})
  endforeach()
  set(${out} ${files} PARENT_SCOPE)
endfunction()

# Plugin cargado con dlopen por el ejecutable
function(seagull_plugin target output_name)
  seagull_collect(srcs ${ARGN})
  add_library(${target} MODULE ${srcs})
  set_target_properties(${target} PROPERTIES OUTPUT_NAME ${output_name})
  #target_link_libraries(${target} PRIVATE pthread)
  target_link_libraries(${target} PRIVATE Threads::Threads)
endfunction()

# ---- Ejecutable principal (BUILD_1) ----
seagull_collect(SEAGULL_EXE_SRCS
  common generator-common generator-core generator-model protocol-frame
  protocol-data protocol-binary protocol-binary-body-with-separator
  protocol-tlv protocol-binary-body-not-interpreted protocol-external
  protocol-text protocol-stats scenario-stats data-log transport-frame
  generator-traffic generator-ext-AgnerLib generator-scenario external-data
  xml-parser generator-stats parser-frame filter-frame command-actions
  generator-control)


find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

set(XMLP ${CMAKE_CURRENT_SOURCE_DIR}/xml-parser)
BISON_TARGET(XmlGrammar ${XMLP}/xml_definition.y ${CMAKE_CURRENT_BINARY_DIR}/y.tab.c
             DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/y.tab.h
             COMPILE_FLAGS "-y")
FLEX_TARGET(XmlLexer ${XMLP}/xml_definition.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c
            COMPILE_FLAGS "-l")
ADD_FLEX_BISON_DEPENDENCY(XmlLexer XmlGrammar)

# los generados son C++ (declaraciones extern con enlace C++)
set_source_files_properties(${BISON_XmlGrammar_OUTPUTS} ${FLEX_XmlLexer_OUTPUTS}
                            PROPERTIES LANGUAGE CXX)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(seagull ${SEAGULL_EXE_SRCS}
               ${BISON_XmlGrammar_OUTPUTS} ${FLEX_XmlLexer_OUTPUTS})

target_compile_definitions(seagull PRIVATE INIT_CALL_FILTER)
set_target_properties(seagull PROPERTIES ENABLE_EXPORTS ON)   # -rdynamic
set_target_properties(seagull PROPERTIES BUILD_RPATH "$ORIGIN") # dlopen encuentra las .so junto al ejecutable
# sin -lfl: xml_definition.l define yywrap() y main.cpp define main()
target_link_libraries(seagull PRIVATE Threads::Threads ${CMAKE_DL_LIBS})

# ---- Plugins (BUILD_2, 5, 6, 7, 12) ----
seagull_plugin(trans_ip    trans_ip    common protocol-frame transport-frame library-trans-ip)
seagull_plugin(parser_xml  parser_xml  common parser-frame filter-frame protocol-frame library-parser-xml)
seagull_plugin(parser_h248 parser_h248 common parser-frame filter-frame protocol-frame library-parser-h248)
seagull_plugin(parser_msrp parser_msrp common parser-frame filter-frame protocol-frame library-parser-msrp)
seagull_plugin(generalmethods _generalmethods common protocol-frame library-general-methods)

# ---- Herramientas en C (BUILD_3, 4) ----
add_executable(csvextract tool-csvextract/csvextract.c)
add_executable(csvsplit   tool-csvsplit/csvsplit.c)


# ---- OpenSSL: usar el del sistema si existe; si no, compilar desde fuente vendored ----
set(SEAGULL_OPENSSL_VERSION "3.5.8" CACHE STRING
    "Versión de OpenSSL a compilar si no se encuentra una instalación en el sistema")
set(SEAGULL_EXT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external-lib-src" CACHE PATH
    "Directorio con tarballs de dependencias externas (modo sin internet)")

if(SEAGULL_BUILD_CRYPTO OR SEAGULL_BUILD_TLS)
  find_package(OpenSSL QUIET)

  if(OpenSSL_FOUND)
    message(STATUS "OpenSSL del sistema encontrado: ${OPENSSL_VERSION} (${OPENSSL_INCLUDE_DIR})")
  else()
    message(STATUS "OpenSSL no encontrado en el sistema; "
                    "se compilará ${SEAGULL_OPENSSL_VERSION} desde fuente vendored")

    set(_openssl_tarball "${SEAGULL_EXT_SRC_DIR}/openssl-${SEAGULL_OPENSSL_VERSION}.tar.gz")
    if(NOT EXISTS ${_openssl_tarball})
      message(FATAL_ERROR
        "No se encontró ${_openssl_tarball}. "
        "Coloca el tarball de OpenSSL ${SEAGULL_OPENSSL_VERSION} en ${SEAGULL_EXT_SRC_DIR} "
        "o ajusta SEAGULL_OPENSSL_VERSION al valor disponible.")
    endif()

    include(ExternalProject)

    set(_openssl_install_dir "${CMAKE_BINARY_DIR}/ext-install/openssl-${SEAGULL_OPENSSL_VERSION}")

    ExternalProject_Add(openssl_ext
      URL               ${_openssl_tarball}
      URL_HASH          SHA256=a8f84a39918ec6415ce765d9b429d313ba97b8143169c172e734b9514464f5b2
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE   # resuelve también el warning de CMP0135
      PREFIX            ${CMAKE_BINARY_DIR}/ext-build/openssl
      INSTALL_DIR       ${_openssl_install_dir}
      BUILD_IN_SOURCE   1
      CONFIGURE_COMMAND <SOURCE_DIR>/config --prefix=<INSTALL_DIR> --openssldir=<INSTALL_DIR>/ssl shared
      BUILD_COMMAND     make -j${CMAKE_BUILD_PARALLEL_LEVEL}
      INSTALL_COMMAND   make install_sw   # instala libs + headers, sin docs/manpages
      BUILD_BYPRODUCTS
        <INSTALL_DIR>/lib64/libssl.so
        <INSTALL_DIR>/lib64/libcrypto.so
    )

    # Creamos los IMPORTED targets manualmente, con los mismos nombres
    # (OpenSSL::SSL / OpenSSL::Crypto) que usa find_package(OpenSSL),
    # para que el resto del CMakeLists.txt no necesite ningún cambio.
    file(MAKE_DIRECTORY "${_openssl_install_dir}/include")  # evita error de CMake: el dir aún no existe en configure-time

    add_library(OpenSSL::Crypto SHARED IMPORTED)
    set_target_properties(OpenSSL::Crypto PROPERTIES
      IMPORTED_LOCATION             "${_openssl_install_dir}/lib64/libcrypto.so"
      INTERFACE_INCLUDE_DIRECTORIES "${_openssl_install_dir}/include")
    add_dependencies(OpenSSL::Crypto openssl_ext)

    add_library(OpenSSL::SSL SHARED IMPORTED)
    set_target_properties(OpenSSL::SSL PROPERTIES
      IMPORTED_LOCATION             "${_openssl_install_dir}/lib64/libssl.so"
      INTERFACE_INCLUDE_DIRECTORIES "${_openssl_install_dir}/include"
      INTERFACE_LINK_LIBRARIES      OpenSSL::Crypto)
    add_dependencies(OpenSSL::SSL openssl_ext)

    set(OpenSSL_FOUND TRUE)
  endif()
endif()
if(SEAGULL_BUILD_CRYPTO)
  seagull_plugin(crypto _crypto common protocol-frame library-crypto)
  #target_link_libraries(crypto PRIVATE OpenSSL::SSL OpenSSL::Crypto dl m)
  target_link_libraries(crypto PRIVATE OpenSSL::SSL OpenSSL::Crypto ${CMAKE_DL_LIBS} m)
  target_compile_definitions(crypto PRIVATE OPENSSL_NO_KRB5)
endif()
if(SEAGULL_BUILD_TLS)
  seagull_plugin(trans_iptls trans_iptls common protocol-frame transport-frame
                 library-trans-ip library-trans-tls)
  #target_link_libraries(trans_iptls PRIVATE OpenSSL::SSL OpenSSL::Crypto dl m)
  target_link_libraries(trans_iptls PRIVATE OpenSSL::SSL OpenSSL::Crypto ${CMAKE_DL_LIBS} m)
  target_compile_definitions(trans_iptls PRIVATE OPENSSL_NO_KRB5)
endif()
# ---- SCTP nativo (BUILD_9): kernel Linux + <netinet/sctp.h>, sin sctplib/socketapi ----
if(SEAGULL_BUILD_SCTP)
  include(CheckIncludeFileCXX)
  check_include_file_cxx(netinet/sctp.h SEAGULL_HAVE_NETINET_SCTP_H)
  if(NOT SEAGULL_HAVE_NETINET_SCTP_H)
    message(FATAL_ERROR "No se encontró <netinet/sctp.h>. Instala lksctp-tools-devel")
  endif()
  seagull_plugin(trans_extsctp trans_extsctp common protocol-frame transport-frame
                 library-trans-ip library-trans-extsctp)
  target_include_directories(trans_extsctp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/library-trans-extsctp)
  target_link_libraries(trans_extsctp PRIVATE ${CMAKE_DL_LIBS})
endif()
