
Docker image
============


Building Rack Docker image
--------------------------

For creating a [Docker image](https://docs.docker.com), **Rack** uses 

- [Dockerfile](./Dockerfile)
- A simplified [install-rack.cnf](install-rack.cnf)  

Then, the image can be created with:

```bash
  cd ./docker
  docker build . --tag rack:local
```
