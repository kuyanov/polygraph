FROM ubuntu:latest
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y git build-essential cmake libboost-system-dev
RUN git clone https://github.com/Tencent/rapidjson.git && \
    cp -r rapidjson/include/rapidjson /usr/local/include/
RUN git clone https://github.com/uNetworking/uWebSockets.git && \
    cd uWebSockets && \
    git submodule init && \
    git submodule update uSockets && \
    git checkout c2f29c86c718e364b5586d103ad592aa396d7604 && \
    make install && \
    cd uSockets && \
    make && \
    cp src/libusockets.h /usr/local/include/ && \
    cp uSockets.a /usr/local/lib

COPY . /polygraph
WORKDIR /polygraph

RUN rm -rf cmake-build-asan && \
    mkdir cmake-build-asan && \
    cd cmake-build-asan && \
    cmake .. -DCMAKE_BUILD_TYPE=ASan && \
    make

ENTRYPOINT [ "bash", "-c", "./cmake-build-asan/scheduler/Scheduler & sleep 0.5 && \
                            ./cmake-build-asan/test/scheduler/TestScheduler && \
                            pkill Scheduler" ]
