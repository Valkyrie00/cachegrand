# Copyright (C) 2018-2022 Vito Castellano
# All rights reserved.
#
# This software may be modified and distributed under the terms
# of the BSD license. See the LICENSE file for details.

FROM valkyrie00/cachegrand:build
LABEL maintainer="d.albano@cachegrand.io"

RUN apt-get install -y ssh rsync gdb tcl jq bc python3-pip \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Needed for tests
RUN python3 -m pip install pyyaml

RUN ( \
    echo 'LogLevel DEBUG2'; \
    echo 'PermitRootLogin yes'; \
    echo 'PasswordAuthentication yes'; \
    echo 'Subsystem sftp /usr/lib/openssh/sftp-server'; \
  ) > /etc/ssh/sshd_config_test_clion \
  && mkdir /run/sshd

RUN useradd -m cachegrand \
  && yes password | passwd cachegrand

RUN usermod -s /bin/bash cachegrand

EXPOSE 22
EXPOSE 6379
EXPOSE 6380

CMD ["/usr/sbin/sshd", "-D", "-e", "-f", "/etc/ssh/sshd_config_test_clion"]