
Docker image
============


Building Rack Docker image
--------------------------

Quick guide
...........

Creating a [Docker](https://docs.docker.com) runtime image with **Rack** can be created with:

```bash
  cd ./docker
  make rack6_runtime
```

Test:

```bash
  cd ./docker
  make rack6_runtime
```


Details
.......

The Docker support in **Rack** consists of a [directory](./) containing 

- [Dockerfile](./Dockerfile) for compiling a ``rack`` executable
- A simple configuration file [install-rack.cnf](install-rack.cnf)
- [Dockerfile-runtime](./Dockerfile-runtime) for creating 
- [Makefile](./Makefile) for easy building with **make** utility.


Then, the image can be created with:

```bash
  cd ./docker
  make rack-full
  make rack-runtime
```
