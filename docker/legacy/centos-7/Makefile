.PHONY: build
build:
	docker build -t localhost/seagull:centos7 .

.PHONY: run
run:
	docker run -i -t --rm --detach --name seagull-centos7 localhost/seagull:centos7 /bin/bash || :

.PHONY: exec
exec:
	docker exec -it seagull-centos7 /bin/bash

.PHONY: stop
stop:
	docker stop seagull-centos7 || :
