FROM ubuntu:18.04 as env

RUN groupdel dialout

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    # sudo \
    build-essential \
    gdb \
    libssl-dev \
    zlib1g-dev \
    dos2unix \
    libc6-dbg \
    valgrind \
    git \
    libomp-dev \
    cmake \
    libcurl4-openssl-dev \
    python3

ARG USER_ID
ARG GROUP_ID
ARG DEP_DIR
ARG SRC_DIR

ENV DEP_DIR=/${DEP_DIR}
RUN echo ${DEP_DIR}
ENV SRC_DIR=/${SRC_DIR}
RUN echo ${SRC_DIR}
RUN echo ${ROOTDIR}

RUN addgroup --gid $GROUP_ID user
RUN adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID user

RUN mkdir -p ${SRC_DIR}
WORKDIR ${SRC_DIR}
RUN git clone https://github.com/dtorban/CppWebServer.git CppWebServer
RUN mkdir -p ${SRC_DIR}/CppWebServer/build

WORKDIR ${SRC_DIR}/CppWebServer/build
RUN cmake -DCMAKE_INSTALL_PREFIX=${DEP_DIR} ..
RUN make install -j

WORKDIR ${DEP_DIR}

RUN find ${install_dir} -type d -exec chmod 775 {} \;
RUN find ${install_dir} -type f -exec chmod 644 {} \;

RUN mkdir -p /home/user
WORKDIR /home/user/repo

USER user