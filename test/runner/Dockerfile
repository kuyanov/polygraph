FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y build-essential git cmake bsdmainutils libboost-system-dev
RUN git clone https://github.com/Tencent/rapidjson.git && \
    cp -r rapidjson/include/rapidjson /usr/local/include/
RUN git clone https://github.com/Forestryks/libsbox.git && \
    mkdir libsbox/build && \
    cd libsbox/build && \
    cmake .. && \
    make && \
    yes | make install

COPY . /polygraph
WORKDIR /polygraph

RUN rm -rf cmake-build-asan && \
    mkdir cmake-build-asan && \
    cd cmake-build-asan && \
    cmake .. \
    -D CMAKE_BUILD_TYPE=ASan \
    -D ENABLE_RUNNER=1 \
    -D ENABLE_TESTING=1 && \
    make

ENTRYPOINT [ "./test/runner/entrypoint.sh" ]
