#!/bin/bash --login

set -e
CUR_DIR=$(pwd)
GIT_ROOT=$(git rev-parse --show-toplevel)
PROJ_ROOT="$GIT_ROOT/connectors/cassandra"
BUILD_DIRECTORY="$PROJ_ROOT/build"
META_DIRECTORY="$PROJ_ROOT/meta"
THIRD_PARTY_DIRECTORY="$PROJ_ROOT/third_party"

# Build deps
if [[ ! -d "$THIRD_PARTY_DIRECTORY" ]]; then
    cd "$META_DIRECTORY"
    source source_ansible_bash
    ansible-playbook -K playbooks/devbox_all.yml
    deactivate
fi

# Build source
if [[ ! -d "$BUILD_DIRECTORY" ]]; then
    mkdir "$BUILD_DIRECTORY"
fi
cd "$BUILD_DIRECTORY"
cmake ..
NPROCS=$(grep -c ^processor /proc/cpuinfo)
make -j$NPROCS
    
# Build Dockerfile
cd $PROJ_ROOT
sudo docker build -t runway/cassandra_sink:0.4.3.1 .
echo "Don't forget to push: sudo docker push runway/cassandra_sink:0.4.3.1"
cd $CUR_DIR
    
