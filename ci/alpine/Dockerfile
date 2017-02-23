FROM alpine:edge
ADD * /scripts/
RUN /scripts/install.sh
CMD mkdir -p /tmp/build && \
    cd /tmp/build && \
    /outside/configure \
        --dev --doc --ignore-coverage && \
    make test && \
    make doc
