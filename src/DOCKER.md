


# Server-side build 

Automated build of an image on a server

Required settings:

| Dockerfile Location | Context Location | Branches/Tags  | Pull Robot |
| ------------------- | ---------------- | -------------  | ---- ----- |
| `/src/Dockerfile`   | `/src`           | `heads/main`   | (None)     |

Tagging options:
- Branch/tag name
- `latest` if default branch
- ${commit_info.short_sha} 


 
# Local build

This section explains how a Docker image is build on command line.

## Basic build commands

An image is constructed by entering `rack/src` directory and issuing:

```
make docker-image
```

Essentially, a command like this one will be run:
```
docker --config docker-conf-<hostname>  build . --build-arg CONF_DIR=docker-conf-<hostname> --file Dockerfile.ubuntu20  --tag rack_ubuntu20:7.79
```



## Proxy settings

Proxy settings are often required for a build to run. Therefore, the process creates a
separare configuration dir, `./docker-conf-${HOSTNAME}` by default.
The directory contains the following files:

* `config.json` – settings for `docker` command
```
{
  "proxies": {
    "default": {
      "httpProxy": "http://proxy.domain.org:8080",
      "httpsProxy": "http://proxy.domain.org:8080",
      "noProxy": "localhost,.something.org"
    }
  }
}
```

* `apt-proxy.conf` – settings for `apt-get` command
```
Acquire::http::Proxy  "http://proxy.domain.org:8080";
Acquire::https::Proxy "http://proxy.domain.org:8080";
```

One may create, then edit and use alternative configuration directories, issued in a `make` parameter as follows:

```
make docker-image DOCKER_CONF_DIR=docker-conf-external
```

## Testing 

After building an image, it is good to test that `rack` command is found and works, at least responses to basic commands.
```
make docker-image-test
```

## Uploading to Quay.io 

(Currently, this part is FMI specific and used only by the developers.)
```
make docker-image-push
```


