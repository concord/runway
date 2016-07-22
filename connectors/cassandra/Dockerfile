FROM concord/runtime_executor:0.4.3

RUN apt-get -y install \
    libboost-context1.55.0

COPY third_party /usr/local
COPY build/cassandra_pumper /usr/local/bin

WORKDIR /
