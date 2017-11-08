.PHONY: help chain dh
NAME := cert

help:  ## Display this help
	@echo If you want to use an existing certifaction auhoritiy, place ca_key.pem
	@echo \(private key of the ca\) and ca_cert.pem \(the public cert of the ca\) in
	@echo this directory.
	@echo
	@echo Otherwise everything will be created
	@echo
	@cat $(MAKEFILE_LIST) | grep -E '^[0-9a-zA-Z_-]+:.*?## .*$$' | sort -k1,1 | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

clean:  ## Remove files
	rm -f *.pem *.srl *.csr

chain: | $(NAME).pem  ## Create a cert-chain (cert.pem) or 'make chain NAME=bla' for bla.pem

dh: | dh.pem  ## Create the dh-parameters

dh.pem:
	openssl dhparam -out dh.pem -5 4096

ca_key.pem:
	openssl genrsa -out ca_key.pem 4096

$(NAME)_key.pem:
	openssl genrsa -out $(NAME)_key.pem 4096

ca_cert.pem: | ca_key.pem
	openssl req -x509 -new -nodes -key ca_key.pem \
        -days 36500 -out ca_cert.pem -sha512

$(NAME).csr: | $(NAME)_key.pem
	openssl req -new -key $(NAME)_key.pem -sha512 \
		-out $(NAME).csr

$(NAME)_pub.pem: | $(NAME).csr ca_cert.pem
	openssl x509 -req -in $(NAME).csr -CA ca_cert.pem \
		-CAkey ca_key.pem -extensions sslclient,sslserver \
		-CAcreateserial -out $(NAME)_pub.pem -days 36500 \
		-sha512

$(NAME).pem: | $(NAME)_pub.pem
	cat $(NAME)_key.pem > $(NAME).pem && \
	cat $(NAME)_pub.pem >> $(NAME).pem && \
	cat ca_cert.pem >> $(NAME).pem || \
	sh -c "rm $(NAME).pem; false"
