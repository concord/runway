cmake_minimum_required (VERSION 2.6)
project (concord_kafka)
set(VERSION 0.1.0)

link_directories (
  ${PROJECT_SOURCE_DIR}/third_party/lib
  /usr/local/lib
  /usr/lib/x86_64-linux-gnu
  )

include_directories (
  SYSTEM
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/third_party/include
  src
  )

set (BASE_CPP_CLIENT_LIBRARIES
  :libbolt_cpp_client.so
  :libglog.so.0
  :libthriftnb-0.9.2.so
  :libthrift-0.9.2.so
  :libthriftz-0.9.2.so
  :libglog.so.0.0.0
  :libgflags.so.2.1.0
  :libpthread.so
  :libdouble-conversion.so.1.0
  :libboost_system.so.1.55.0
  :libevent.a
  :libglog.so.0
  :libstdc++.so.6
  :libssl.so.1.0.0
  :libcrypto.so.1.0.0
  :libgflags.so.2
  :libunwind.so.8
  :libpthread.so.0
  :libz.so.1.2.8
  )

set(BASE_FLAGS
  "-fPIC"
  "-fuse-ld=gold"
  "-Wall"
  "-Werror"
  "-Wextra"
  "-Wformat"
  "-pedantic"
  "-Wmissing-braces"
  "-Wparentheses"
  "-Wpointer-arith"
  "-Wformat-security"
  "-Wunused"
  "-Wno-unused-parameter"
  "-Wcast-align"
  "-Wmissing-field-initializers"
  "-O2"
  )

set (CXX_FLAG_LIST
  ${BASE_FLAGS}
  "-std=c++1y"
  "-ftemplate-depth-256"
  "-Wnon-virtual-dtor"
  "-Woverloaded-virtual"
  "-Wsign-promo"
  "-Wreorder"
  "-Wdelete-non-virtual-dtor"
  )

function (join VALUES SEP OUTPUT)
  string (REPLACE ";" "${SEP}" _TMP_STR "${VALUES}")
  set (${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
endfunction ()

set (CMAKE_CXX_COMPILER g++-5)
set (CMAKE_C_COMPILER gcc-5)
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -v" )
join ("${CXX_FLAG_LIST}" " " CMAKE_CXX_FLAGS)
join ("${BASE_FLAGS}" " " CMAKE_C_FLAGS)

set (KAFKA_SOURCES
  ${PROJECT_SOURCE_DIR}/lib/HighLevelKafkaConsumer.cc
  ${PROJECT_SOURCE_DIR}/lib/HighLevelKafkaProducer.cc
  )

add_library(concord_kafka STATIC
  ${KAFKA_SOURCES}
  )

add_executable (
  kafka_source
  ${PROJECT_SOURCE_DIR}/src/kafka_source.cc
  )
target_link_libraries (
  kafka_source
  ${BASE_CPP_CLIENT_LIBRARIES}
  concord_kafka
  rdkafka.a
  rdkafka++.a
  :libsasl2.so.2.0.25  
  )

add_executable (
  kafka_sink
  ${PROJECT_SOURCE_DIR}/src/kafka_sink.cc
  )
target_link_libraries (
  kafka_sink
  ${BASE_CPP_CLIENT_LIBRARIES}
  concord_kafka
  rdkafka.a
  rdkafka++.a
  :libsasl2.so.2.0.25  
  )
