
Building Rack Docker image
==========================

Quick guide
-----------

Creating a [Docker](https://docs.docker.com) runtime image with **Rack** can be created with:

```bash
  cd ./docker
  make rack6_runtime
```

Testing:

```bash
  make test
```


Slow guide
----------

The Docker support in **Rack** consists of a [directory](./) containing 

- [Makefile](./Makefile) for easy building with ``make`` utility
- [Dockerfile](./Dockerfile) for compiling a ``rack`` executable
- A simple configuration file [install-rack.cnf](install-rack.cnf)
- [Dockerfile.runtime](./Dockerfile.runtime) for compiling an image with the executable, excluding build dependencies

The build steps programmed in Makefile can be listed with a plain ``make`` command. Two variables –
``$TMPFILE`` ``$TAG`` – can be tuned upon invocation. For example: ``make TMPFILE=rack-$$ TAG=test rack6_runtime``.

The runtime image should be built easily with the make commands shown above but for debugging,
the build can be performed step by step as follows.

Building the initial, full image containing all the dependencies:
```bash
  docker build . --file Dockerfile --tag rack6_full:latest
```

Exporting the executable from the container to the working dir:
```bash
  touch /tmp/rack.tmp
  docker run --mount src=/tmp,dst=/tmp,type=bind -t rack6_full cp -fv /rack/rack/Release/rack /tmp/rack.tmp
  mv /tmp/rack.tmp bin/rack # or cp
  chmod a+rx bin/rack
```

Building the final (runtime) image
```bash
  docker build . --file Dockerfile.runtime --tag rack6_runtime:latest
```

Testing a runtime container
```bash
  docker run rack6_runtime rack --help
  docker run -it rack6_runtime /bin/bash
  # rack --help
  # ...
```