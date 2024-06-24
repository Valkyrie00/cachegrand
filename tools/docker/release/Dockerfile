# Copyright (C) 2018-2022 Daniele Salvatore Albano
# All rights reserved.
#
# This software may be modified and distributed under the terms
# of the BSD license. See the LICENSE file for details.

FROM valkyrie00/cachegrand:build
LABEL maintainer="d.albano@cachegrand.io"

# Install the required deps
RUN apt-get update -y \
    && apt-get upgrade -y \
    && apt-get install -y --no-install-recommends \
        libnuma1 libyaml-0-2 libcurl4 libmbedtls14 libatomic1 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN cd /root \
    && mkdir build \
    && cd build \
    && git clone https://github.com/danielealbano/cachegrand.git \
    && cd cachegrand \
    && git submodule update --init --recursive \
    && mkdir cmake-build-release \
    && cd cmake-build-release \
    && cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_HASH_ALGORITHM_T1HA2=1 \
    && make cachegrand-server

COPY /root/build/cachegrand/cmake-build-release/src/cachegrand-server /usr/bin

EXPOSE 6379

CMD ["/usr/bin/cachegrand-server", "-c" ,"/etc/cachegrand/cachegrand.yaml"]
