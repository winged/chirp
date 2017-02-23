FROM alpine:edge
ADD * /scripts/
RUN /scripts/install.sh
CMD cd /outside/build && \
    ../configure --dev --doc --ignore-coverage && \
    make test && \
    make doc
