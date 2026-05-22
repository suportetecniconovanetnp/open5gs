Docker running example
===========================================
* Development
  
  `$ docker compose run dev`

* Run WebUI
  
  `$ docker-compose up webui`

* Test

  `$ docker compose run test`

* Test(ubuntu:focal)

  `$ TAG=focal docker compose run test`

* Development(fedora:latest)

  `$ DIST=fedora docker compose run dev`

* All Test with All Environment
  
  `$ ./check.sh`

* Runtime

  `$ docker compose run run`

Portainer Stack (NF split)
===========================================
* Ready-to-use stack file with separate services (`mme`, `smf`, `sgwc`, `sgwu`, `hss`, `pcrf`, `upf`, `webui`, `mongodb`):

  `docker/portainer-stack.yml`

* Build required images first:

  ```
  $ cd docker
  $ export USER=open5gs
  $ export DIST=ubuntu
  $ export TAG=latest
  $ export BASE_IMAGE_TAG=latest
  $ export BUILD_IMAGE_TAG=latest
  $ docker compose build --no-cache base build webui
  ```

* Suggested tags used by the Portainer stack:
  * `open5gs/ubuntu-latest-open5gs-base:latest`
  * `open5gs/ubuntu-latest-open5gs-build:latest`
  * `open5gs/open5gs-webui:latest`

Build Images Directly with `docker build`
===========================================

Use separate values for:

* the Ubuntu/Distro selector embedded in the image name (`DIST`/`TAG`)
* the Docker tag of the base image (`BASE_IMAGE_TAG`)
* the Docker tag of the final build image (`BUILD_IMAGE_TAG`)

Example:

```
$ docker build \
    -t open5gs/ubuntu-latest-open5gs-base:20260505 \
    --build-arg dist=ubuntu \
    --build-arg tag=latest \
    docker/ubuntu/latest/base
```

```
$ docker build \
    -f docker/build/Dockerfile \
    -t open5gs/ubuntu-latest-open5gs-build:20260505 \
    --build-arg base_image_name=open5gs/ubuntu-latest-open5gs-base \
    --build-arg base_tag=20260505 \
    .
```

The build image no longer infers the base image from `username`, `dist`, and `tag`. Pass the base image name and its Docker tag explicitly.

For OpenSUSE Build Service Release
===========================================

* Build Package
  
  ```
  $ ./build-aux/git-version-gen . > .tarball-version
  $ dpkg-source -b .
  $ rm -f .tarball-version
  ```

* Get Release Key
  
  ```
  $ wget http://download.opensuse.org/repositories/home:/acetcom:/open5gs:/latest/xUbuntu_18.04/Release.key
  $ sudo apt-key add Release.key
  ```

* Setup Repository
  ```
  $ sudo sh -c "echo 'deb https://download.opensuse.org/repositories/home:/acetcom:/open5gs:/latest/xUbuntu_18.04/ ./' > /etc/apt/sources.list.d/open5gs-latest.list"
  $ sudo apt-get update
  ```
  
For Launchpad Release
===========================================

* Run Docker
  
  ```
  $ docker-compose run dev
  ```

* Setup Debian Environment

```
export DEBFULLNAME='Sukchan Lee'
export DEBEMAIL='acetcom@gmail.com'
```

* Transfer GPG key

```
  $ gpg --export-secret-keys --armor --output private.asc
  $ gpg --import private.asc
  $ gpg --export > public.asc
  $ gpg --import public.asc
  ```

* Version Change
  
  ```
  $ cat ./meson.build
  $ cat ./webui/package.json
  $ cat ./webui/package-lock.json
  $ cat ./docs/assets/webui/install
  ```

* New NF
  ```
  $ cat debian/control
  $ cat configs/systemd/open5gs-scpd.service.in
  $ cat configs/logrotate/open5gs.in
  $ cat configs/newsyslog/open5gs.conf.in
  ```

* Upload OpenBuildService
  
  ```
  $ dch -i
  $ meson subprojects download freeDiameter prometheus-client-c
  $ debuild -S -uc -us -d
  $ osc co home:acetcom:open5gs latest
  $ cd home\:acetcom\:open5gs/latest/
  $ cp ~/git/open5gs_1.0.0.* .
  $ osc ci -m "Update it"
  ```

* Upload LaunchPad
  
  ```
  $ dch -i
  $ meson subprojects download freeDiameter prometheus-client-c
  $ debuild -S -d
  $ dput ppa:open5gs/latest *.source.changes
  ```

* Tagging
  
  ```
  $ git tag v1.x.x -a
  ```

* Build package
  
  ```
  $ dpkg-buildpackage -d
  ```
