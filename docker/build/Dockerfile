ARG base_image_name=acetcom/ubuntu-latest-open5gs-base
ARG base_tag=latest
FROM ${base_image_name}:${base_tag}

LABEL org.opencontainers.image.authors="Sukchan Lee <acetcom@gmail.com>"

WORKDIR /open5gs
COPY docker/build/setup.sh /root
COPY ./ /open5gs

ADD https://github.com/ufoscout/docker-compose-wait/releases/download/2.9.0/wait /wait
RUN chmod +x /wait

# Some source snapshots miss prometheus-client-c/README.md, but CPack expects it.
RUN if [ ! -f /open5gs/subprojects/prometheus-client-c/README.md ]; then \
      printf "prometheus-client-c\n" > /open5gs/subprojects/prometheus-client-c/README.md; \
    fi

RUN meson build && ninja -C build install
