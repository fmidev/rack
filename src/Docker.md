


# Automated build of an image on a server

Required settings:
```

```
 
# Building an image on command line

An image is constructed by entering `rack/src` directory and issuing:

```
make docker-image
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

```
make docker-image-test
```

## Uploading to Quay.io 

```
make docker-image-push
```


