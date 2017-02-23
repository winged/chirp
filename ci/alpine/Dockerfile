FROM alpine:edge
ADD * /scripts/
RUN /scripts/install.sh
CMD /outside/configure \
        --dev --doc --ignore-coverage && \
    make test && \
    make doc
