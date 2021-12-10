# COMPASS in MapD (version: 3.6.1)
This document contains all the instructions to replicate COMPASS on a docker container.
- [Docker Engine - Community](https://www.docker.com/docker-community) (client version: 20.10.7, server version: 19.03.8)
- The docker image with pre-installed MapD can be pulled from [DockerHub](https://hub.docker.com/r/shusson/mapd-cuda/tags)


Commands on the host machine:
- `sudo apt-add-repository multiverse`
- `sudo apt update`
- `sudo apt install nvidia-modprobe`
- `dpkg -l | grep nvidia-docker`<br/>
  if nvidia-docker2: `docker run -it --name compass_docker --gpus device=0 --runtime=nvidia --shm-size 64gb shusson/mapd-cuda:2966fac`<br/>
  if nvidia-docker1: `docker run -d -it --name compass_docker --gpus device=0 --shm-size 64gb shusson/mapd-cuda:2966fac`
- `exit`
- `docker start compass_docker`
- `docker cp path_to_your_source_code/mapd-core/ compass_docker:/home/`
- `docker exec -it compass_docker bash`

Commands on the docker machine:
- `apt update`
- `apt install apt-transport-https sudo screen time vim`
- `apt install -y libbz2-dev libarchive-dev`

Installing `arrow` dependency:
- `ARROW_BOOST_USE_SHARED="ON"`
- `ARROW_VERSION=apache-arrow-0.7.1`
- `wget --continue https://github.com/apache/arrow/archive/$ARROW_VERSION.tar.gz`
- `tar xvf $ARROW_VERSION.tar.gz`
- `mkdir -p arrow-$ARROW_VERSION/cpp/build`
- `pushd arrow-$ARROW_VERSION/cpp/build`
- `cmake -DCMAKE_BUILD_TYPE=Release -DARROW_BUILD_SHARED=ON -DARROW_BUILD_STATIC=ON -DARROW_BUILD_TESTS=OFF -DARROW_BUILD_BENCHMARKS=OFF -DARROW_WITH_BROTLI=OFF -DARROW_WITH_ZLIB=OFF -DARROW_WITH_LZ4=OFF -DARROW_WITH_SNAPPY=OFF -DARROW_WITH_ZSTD=OFF -DCMAKE_INSTALL_PREFIX=$PREFIX -DARROW_BOOST_USE_SHARED=${ARROW_BOOST_USE_SHARED:="OFF"} ..`
- `make -j $(nproc)`
- `make install`
- `popd`

Paths that are required for MapD to compile the source code:
- PATH: /usr/local/cuda/bin:/usr/local/mapd-deps/bin:/build/mapd-core/build/bin:/usr/local/nvidia/bin:/usr/local/cuda/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
- LD_LIBRARY_PATH: /usr/local/cuda/lib64:/usr/lib/jvm/default-java/jre/lib/amd64/server:/usr/local/mapd-deps/lib:/usr/local/mapd-deps/lib64:/usr/local/nvidia/lib:/usr/local/nvidia/lib64

Compiling the source code:
- `cd mapd-core`
- `mkdir build`
- `cd build`
- `cmake -DCMAKE_BUILD_TYPE=debug ..`
- `make -j $(nproc)`
- `mkdir /home/data`
- `./bin/initdb /home/data`

You can find more details in [Filter Push-Down extension for MapD](https://github.com/junhyungshin/mapd-core-fpd).

Runing the server and client:
- Run the server: `./bin/mapd_server --num-gpus 1 --start-gpu 0 -p 7071 --http-port 7070 --data /home/data/`
- Run the client on the `mapd` database (default): `./bin/mapdql mapd -u mapd --port 7071 -p HyperInteractive`
- `CREATE DATABASE imdb;`
- `\q`
- running the client on the imdb database: `./bin/mapdql imdb -u mapd --port 7071 -p HyperInteractive`
- Choose GPU or CPU mode: `\gpu` or `\cpu`
- Turn on COMPASS: `\fpd`
- Paste a query.
- Stop client: `\q`
- Stop server: `ctrl + c` on keyboard


# Hyper parameters in COMPASS with default values:

Parameters related to Fast-AGMS sketches (make sure the sketch templates has the same bucket size):
- CAT_SKETCH_BUCKETS (unsigned) = 1021. The number of bucket size.
- CAT_SKETCH_ROWS (unsigned) = 11. The number of basic Fast-AGMS sketches.

Parameters related to Fast-AGMS sketch templates:
- PRE_PROCESSING (false=0 true=1) = false. Enable when the sketch templates are creaated.
- PATH_TO_SKETCHES. Path to the sketch templates and seeds.
- PATH_TO_CATALOG_SK = `sketch_templates_1021_11.txt`. File name that the sketch templates are stored.
- PATH_TO_CATALOG_SEEDS = `sketch_templates_seeds_1021_11.txt`. File name that the sketch seeds are stored.

Parameters related to push-down:
- PUSH_DOWN_MIN_TABLE_SIZE_SK (unsigned) = 200.
- PUSH_DOWN_MAX_SELECTIVITY_SK (float) = 0.05.
- PUSH_DOWN_MAX_SIZE (unsigned) = 10000.

You can find more details about these three parameters in [Filter Push-Down extension for MapD](https://github.com/junhyungshin/mapd-core-fpd).

Parameters related to GPU for parallel sketch building:
- block_size (unsigned) = 1024
- grid_size (unsigned) = 26
- warp_size (int8_t) = 32

Parameters related to the join graph enumeration:
- NODE_TRAVERSE_BOUND = 1. The number of traversals per node in a given join graph. This parameter can be also adjusted in [`compass/mapd-core/QueryEngine/RelAlgExecutor.cpp`](https://github.com/yizenov/compass_optimizer/blob/main/compass/mapd-core/QueryEngine/RelAlgExecutor.cpp).


You can find all the parameters settings in [`compass/mapd-core/Catalog/Catalog.h`](https://github.com/yizenov/compass_optimizer/blob/main/compass/mapd-core/Catalog/Catalog.h).


# Information about software/package/driver versions:

Host machine package versions:
- OS: Ubuntu 20.04
- GPU driver: 470.63.01
- CUDA version (nvidia-smi): 11.4
- CUDA version (nvcc): 10.1.243
- Maven: 3.6.3
- OpenJDK: 11.0.11
- Go: 1.13.8
- perl: 5.30.0
- c++, g++, gcc: 9.3.0
- clang: 10.0.0
- boost: 1.71.0
- llvm: 10.0.0
- python3: 3.8.10
- python2: 2.7.18

Docker Image package versions:
- OS: Ubuntu 16.04
- GPU driver: 470.63.01
- CUDA version (nvidia-smi): 11.4
- CUDA version (nvcc): 8.0.61
- Maven: 3.3.9
- OpenJDK: 1.8.0_131
- Go: 1.6.2
- perl: 5.22.1
- c++, g++, gcc: 5.4.0
- clang: 3.8.0
- boost: 1.58.0
- llvm: 3.8.0
- python3: not installed
- python2: 2.7.12

MapD dependencies:
- Thrift: 0.10.0
- c-blosc: 1.11.3
- Folly: 2017.04.10.00
- Bison: 2.5.1
- Arrow: 0.7.1 (needs to be installed)
- Glbinding
