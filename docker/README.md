Each directory contains a _Dockerfile_ and a _Makefile_. Use ``make`` to build, run, exec, and stop container.

# Usage

    $ make build

Builds a Docker image that contains Seagull ready to run. It uses a multi-stage Docker build. All build layers are
abandoned after a successful build and only the image for running Seagull remains. This image can be used to run a
container to use Seagull.

    $ make run
    $ make exec
    $ make stop

These ``make`` targets control the lifecycle of a container based on the image created by ``make build``.
