.PHONY: build
build:
	docker build -t localhost/seagull:ubuntu1804 .

.PHONY: run
run:
	docker run -i -t --rm --detach --name seagull-ubuntu1804 localhost/seagull:ubuntu1804 /bin/bash || :

.PHONY: exec
exec:
	docker exec -it seagull-ubuntu1804 /bin/bash

.PHONY: stop
stop:
	docker stop seagull-ubuntu1804 || :
