FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y build-essential git cmake libboost-system-dev && \
    apt-get clean

COPY . /polygraph
WORKDIR /polygraph

ENV BUILD_DIR=cmake-build-asan

RUN rm -rf $BUILD_DIR && \
    mkdir $BUILD_DIR && \
    cd $BUILD_DIR && \
    cmake .. \
    -D CMAKE_BUILD_TYPE=ASan \
    -D ENABLE_SCHEDULER=1 \
    -D ENABLE_TESTING=1 && \
    make

ENTRYPOINT [ "./test/scheduler/test.sh" ]
