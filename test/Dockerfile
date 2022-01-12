FROM ubuntu:latest
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get upgrade -y && apt-get install -y git build-essential cmake libboost-system-dev libuv1.dev

COPY . /PolyGraph
WORKDIR /PolyGraph

RUN rm -rf cmake-build-asan && mkdir cmake-build-asan && cd cmake-build-asan && cmake .. -DCMAKE_BUILD_TYPE=ASan && make

ENTRYPOINT [ "bash", "-c", "cd test && ../cmake-build-asan/test/TestMasterNode" ]
