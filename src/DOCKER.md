


# Automated build of an image on a server

Required settings:
```

```

# Building an image on command line

```
make docker
```

The process creates a separare configuration dir, `./docker-conf-${HOSTNAME}` by default.
The directory contains files

* `config.json` – settings for docker command
* `apt-proxy.conf` – settings for docker command

If the build environment has proxy settings, defined as variables `http_proxy` and `https_proxy`,
the process stores them in these files, appearing as follows:

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

```
Acquire::http::Proxy  "http://proxy.domain.org:8080";
Acquire::https::Proxy "http://proxy.domain.org:8080";
```

One may create, then edit and use alternative configuration directories, issued in a `make` parameter as follows:

```
make docker DOCKER_CONF_DIR=docker-conf-external
```


