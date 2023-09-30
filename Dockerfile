FROM alpine:3.14 AS build

ENV HOME /root

WORKDIR /root/src

# install necessary packages
RUN apk --update add --no-cache \
    build-base \
    python3 \
    cmake \
    curl \
    unzip \
    linux-headers \
    perl

# ensure pip is installed
RUN python3 -m ensurepip --upgrade

# install conan
RUN python3 -m pip install conan

# install b2 for boost
RUN curl -OLs https://github.com/bfgroup/b2/archive/refs/tags/4.9.0.zip && \
    unzip 4.9.0.zip && \
    cd b2-4.9.0 && \
    ./bootstrap.sh && \
    ./b2 install

# configure conan
RUN conan profile detect --force

# copy source files
COPY . /root/src

COPY ./CMakeLists.txt /root/src/CMakeLists.txt

# build the project
RUN make build

RUN cd cmake.bld/Linux/full

ENTRYPOINT ["make", "test"]
