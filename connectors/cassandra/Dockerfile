FROM concord/runtime_executor:0.4.3.2

RUN apt-get -y install \
    libboost-context1.55.0

COPY meta/tmp/include /usr/include
COPY meta/tmp/lib /usr/lib
COPY build/cassandra_pumper /usr/local/bin

WORKDIR /
