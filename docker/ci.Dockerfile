FROM bitnami/minideb:bullseye

MAINTAINER Vladimir Shcherba <abrehchs@gmail.com>

# add some required tools
RUN apt-get update && \
    apt-get install --no-install-recommends -y \
        gpg \
        gpg-agent \
        wget \
        python3 \
        python3-pip \
        python3-setuptools \
        software-properties-common \
        curl && \
        rm -rf /var/lib/apt/lists/*

# add repos for llvm and newer gcc and install docker
RUN apt-get update && apt-get install --no-install-recommends -y \
        build-essential \
        gcc-12 \
        g++-12 \
        llvm-14-dev \
        clang-14 \
        clang-tidy-14 \
        clang-format-14 \
        make \
        git \
        ccache \
        lcov \
        zlib1g-dev \
        unzip && \
    rm -rf /var/lib/apt/lists/*

# install cmake and dev dependencies
RUN python3 -m pip install --no-cache-dir --upgrade pip
RUN pip3 install --no-cache-dir cmake scikit-build requests gitpython gcovr pyyaml

# install sonar cli
ENV SONAR_CLI_VERSION=4.1.0.1829
RUN set -e; \
    mkdir -p /opt/sonar; \
    curl -L -o /tmp/sonar.zip https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-${SONAR_CLI_VERSION}-linux.zip; \
    unzip -o -d /tmp/sonar-scanner /tmp/sonar.zip; \
    mv /tmp/sonar-scanner/sonar-scanner-${SONAR_CLI_VERSION}-linux /opt/sonar/scanner; \
    ln -s -f /opt/sonar/scanner/bin/sonar-scanner /usr/local/bin/sonar-scanner; \
    rm -rf /tmp/sonar*

# set env
ENV LLVM_ROOT=/usr/lib/llvm-14
ENV LLVM_DIR=/usr/lib/llvm-14/lib/cmake/llvm/
ENV PATH=${LLVM_ROOT}/bin:${LLVM_ROOT}/share/clang:${PATH}
ENV CC=gcc-12
ENV CXX=g++-12

# set default compilers and tools
RUN update-alternatives --install /usr/bin/python       python       /usr/bin/python3               90 && \
    update-alternatives --install /usr/bin/clang-tidy   clang-tidy   /usr/bin/clang-tidy-14         90 && \
    update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-14       90 && \
    update-alternatives --install /usr/bin/clang        clang        /usr/lib/llvm-14/bin/clang-14  90 && \
    update-alternatives --install /usr/bin/clang++      clang++      /usr/bin/clang++-14            90 && \
    update-alternatives --install /usr/bin/gcc          gcc          /usr/bin/gcc-12                90 && \
    update-alternatives --install /usr/bin/g++          g++          /usr/bin/g++-12                90 && \
    update-alternatives --install /usr/bin/gcov         gcov         /usr/bin/gcov-12               90
