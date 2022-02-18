FROM ubuntu:latest
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get upgrade -y && apt-get install -y git build-essential cmake libboost-system-dev libuv1.dev

COPY . /polygraph
WORKDIR /polygraph

RUN rm -rf cmake-build-asan && mkdir cmake-build-asan && cd cmake-build-asan && cmake .. -DCMAKE_BUILD_TYPE=ASan && make
RUN rm -rf cmake-build-tsan && mkdir cmake-build-tsan && cd cmake-build-tsan && cmake .. -DCMAKE_BUILD_TYPE=TSan && make

ENTRYPOINT [ "bash", "-c", "cd masternode && ../cmake-build-asan/test/masternode/TestMasterNode && \
                                             ../cmake-build-tsan/test/masternode/TestMasterNode" ]
