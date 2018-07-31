.PHONY: build
build:
	docker build -t localhost/seagull:ubuntu1604 .

.PHONY: run
run:
	docker run -i -t --rm --detach --name seagull-ubuntu1604 localhost/seagull:ubuntu1604 /bin/bash || :

.PHONY: exec
exec:
	docker exec -it seagull-ubuntu1604 /bin/bash

.PHONY: stop
stop:
	docker stop seagull-ubuntu1604 || :
