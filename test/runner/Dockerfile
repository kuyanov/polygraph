FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y build-essential git cmake bsdmainutils libboost-program-options-dev
RUN git clone https://github.com/kuyanov/libsbox.git && \
    mkdir libsbox/build && \
    cd libsbox/build && \
    cmake .. && \
    make && \
    yes | make install

COPY . /polygraph
WORKDIR /polygraph

ENV BUILD_DIR=cmake-build-asan

RUN rm -rf $BUILD_DIR && \
    cmake . -B $BUILD_DIR -D CMAKE_BUILD_TYPE=ASan -D ENABLE_TESTING=1 && \
    cmake --build $BUILD_DIR && \
    cmake --install $BUILD_DIR

ENTRYPOINT [ "bash", "-c", "./test/runner/test.sh $BUILD_DIR || (cat /var/log/polygraph/common.log && exit 1)" ]
