FROM alpine:3.20

# Core C++ toolchain + common dev tools.
# - build-base: gcc, g++, make, libc-dev, etc.
# - clang/lld: alternative compiler + fast linker (optional but handy)
# - cmake, ninja: build systems
# - gdb: debugging (VS Code C++ debugger needs this; lldb is an alternative)
# - git, bash, curl: general dev ergonomics
# - musl-dbg: debug symbols for musl libc, improves gdb backtraces
RUN apk add --no-cache \
    build-base \
    clang \
    lld \
    lldb \
    cmake \
    ninja \
    gdb \
    musl-dbg \
    git \
    bash \
    curl \
    linux-headers \
    pkgconf \
    vim

RUN apk add --no-cache libx11-dev libxext-dev xvfb

# Non-root user so files created in the container match your host UID/GID
# reasonably well and VS Code doesn't complain about running as root.
ARG USERNAME=dev
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN addgroup -g $USER_GID $USERNAME \
    && adduser -D -u $USER_UID -G $USERNAME -s /bin/bash $USERNAME

USER $USERNAME
WORKDIR /workspace