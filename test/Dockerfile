FROM ubuntu:latest
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get upgrade -y && apt-get install -y git build-essential cmake libboost-system-dev

WORKDIR /PolyGraph

ENTRYPOINT [ "bash", "-c", "rm -rf build && \
                            mkdir build && \
                            cd build && \
                            cmake .. -DCMAKE_BUILD_TYPE=ASan && \
                            make && \
                            cd ../test && \
                            ../build/test/TestMasterNode" ]
