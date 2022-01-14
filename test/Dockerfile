FROM ubuntu:latest AS base

RUN set -ex; \
    apt-get update; \
    apt-get install -y build-essential libcap-dev

FROM base as build

RUN set -ex; \
    mkdir -p /usr/src

COPY . /usr/src/ctuna

RUN set -ex; \
    cd /usr/src/ctuna; \
    make; \
    make install

FROM base as run

COPY --from=build /usr/local/bin /usr/local/bin

COPY test/dockerentry.sh /usr/local/bin

ENTRYPOINT ["dockerentry.sh"]
