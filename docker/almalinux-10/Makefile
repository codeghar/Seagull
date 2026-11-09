.RECIPEPREFIX = >
ENGINE     ?= podman
IMAGE      ?= localhost/seagull:almalinux10
IMAGE_TEST ?= localhost/seagull:almalinux10-test
NAME       ?= seagull-almalinux10
REPO       := $(shell git rev-parse --show-toplevel)
COMMIT     := $(shell git -C $(REPO) rev-parse --short HEAD)
CTX        := $(CURDIR)/.ctx

.PHONY: context build test run exec stop clean

context:
> rm -rf $(CTX) && mkdir -p $(CTX)
> git -C $(REPO) archive --format=tar HEAD seagull/trunk/src | tar -x -C $(CTX)

build: context
> $(ENGINE) build -f $(CURDIR)/Dockerfile --target runtime --build-arg SEAGULL_COMMIT=$(COMMIT) -t $(IMAGE) $(CTX)

test: context
> $(ENGINE) build -f $(CURDIR)/Dockerfile --target test -t $(IMAGE_TEST) $(CTX)
> $(ENGINE) run --rm $(IMAGE_TEST) tests/verify.sh /src/build/bin

run:
> $(ENGINE) run -i -t --rm --detach --name $(NAME) $(IMAGE) /bin/bash || :

exec:
> $(ENGINE) exec -it $(NAME) /bin/bash

stop:
> $(ENGINE) stop $(NAME) || :

clean:
> rm -rf $(CTX)
