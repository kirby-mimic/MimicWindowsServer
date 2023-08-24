# Building with CMake

## Dependencies

For a list of dependencies, please refer to [vcpkg.json](vcpkg.json).

## Build

## Install vcpkg
```sh
sudo apt-get update
sudo apt-get upgrade -y
sudo apt-get install -y git curl zip unzip tar pkg-config clang-tidy cppcheck cmake build-essential ninja-build
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh -disableMetrics
./vcpkg/vcpkg integrate install
Applied user-wide integration for this vcpkg root.
CMake projects should use: "-DCMAKE_TOOLCHAIN_FILE=${HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

## Add VCPKG_ROOT to ~/.bashrc
```sh
# Example
export VCPKG_ROOT=${HOME}/vcpkg
```
For arm64, this must also be set in ~/.bashrc:
```
export VCPKG_FORCE_SYSTEM_BINARIES=1
```
## Source ~/.bashrc
```sh
source ~/.bashrc
echo $VCPKG_ROOT
/home/user/vcpkg
```


## Git Credential Manager
To access the protobufs repository, you will need to configure a github personal access token.
- [Create a GitHub Personal Access Token](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/managing-your-personal-access-tokens)
- [Install Git Credential Manager](https://github.com/git-ecosystem/git-credential-manager/blob/release/docs/install.md)

## For Linux, it is recommended to use pass
[Create GitHub Classic Personal Access Token](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/managing-your-personal-access-tokens)
```sh
sudo apt-get install pass
gpg --list-secret-keys --keyid-format LONG
pass init <key-id>
git config --global credential.credentialStore gpg
```

# Build MimicWindowsServer
```sh
git clone git@github.com:kirby-mimic/MimicWindowsServer.git
cd MimicWindowsServer
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

## Build MimicWindowsServer
```sh
git clone git@github.com:kirby-mimic/MimicWindowsServefr.git
cd MimicWindowsServer
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=${HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake --preset=ci-ubuntu
cmake --build build --config Release
```

## Build minispyserver docker container
```sh
sudo groupadd docker
sudo usermod -aG docker $USER
newgrp docker
docker build . -t mimicwindowsserver:latest
```

## Running docker
```sh
docker compose -f .\docker-compose-core.yml pull --ignore-pull-failures
docker compose -f .\docker-compose-core.yml up
[+] Pulling 34/29
 ✘ minispy-server Error                                                                                                                                                                                                         1.3s
 ✔ jaeger-collector 4 layers [⣿⣿⣿⣿]      0B/0B      Pulled                                                                                                                                                                      8.0s
 ✔ elasticsearch 8 layers [⣿⣿⣿⣿⣿⣿⣿⣿]      0B/0B      Pulled                                                                                                                                                                     9.8s
 ✔ kibana 14 layers [⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿]      0B/0B      Pulled                                                                                                                                                                    15.8s
 ✔ jaeger-query 1 layers [⣿]      0B/0B      Pulled                                                                                                                                                                             7.8s
docker compose -f .\docker-compose.yml up
```

## It may be necessary to restart our containers (for now)
This is because it takes a long time for the elastic search container to become ready.
```sh
 docker compose -f .\docker-compose.yml restart minispy-server
 docker logs -f minispy-server
```

## Elasticsearch Dashboard 
- [http://localhost:9200/](http://localhost:9200/)
- [http://localhost:9200/minispy_index/_search?pretty](http://localhost:9200/minispy_index/_search?pretty)
- [http://localhost:9200/_aliases?pretty=true](http://localhost:9200/_aliases?pretty=true)
```json
{
  ".security-7" : {
    "aliases" : {
      ".security" : { }
    }
  },
  "jaeger-service-2023-06-29" : {
    "aliases" : { }
  },
  "jaeger-span-2023-06-29" : {
    "aliases" : { }
  }
}
```

- [http://localhost:9200/_cat/indices?v](http://localhost:9200/_cat/indices?v)
```json
health status index                     uuid                   pri rep docs.count docs.deleted store.size pri.store.size
green  open   .geoip_databases          XPgR9qFsTlOvaZaugfh_OQ   1   0         42            0       40mb           40mb
green  open   .security-7               Nk3AiGisSRGuE5sI3BW_Pg   1   0          7            0     25.6kb         25.6kb
yellow open   jaeger-span-2023-06-29    kBSWCdjeQKq7G4Dx7FPVCg   5   1       1902            0      210kb          210kb
yellow open   jaeger-service-2023-06-29 mWG2b37ZRj2SM_Os4Ekcqw   5   1          2            0      8.5kb          8.5kb
```

## Kibana Dashboard 
- [http://localhost:5601/app/home#/](http://localhost:5601/app/home#/)

## Jaeger Dashboard
- [http://localhost:16686/search](http://localhost:16686/search)

This project doesn't require any special command-line flags to build to keep
things simple.

Here are the steps for building in release mode with a single-configuration
generator, like the Unix Makefiles one:

```sh
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

Here are the steps for building in release mode with a multi-configuration
generator, like the Visual Studio ones:

```sh
cmake -S . -B build
cmake --build build --config Release
```

### Building with MSVC

Note that MSVC by default is not standards compliant and you need to pass some
flags to make it behave properly. See the `flags-windows` preset in the
[CMakePresets.json](CMakePresets.json) file for the flags and with what
variable to provide them to CMake during configuration.

### Building on Apple Silicon

CMake supports building on Apple Silicon properly since 3.20.1. Make sure you
have the [latest version][1] installed.

## Install

This project doesn't require any special command-line flags to install to keep
things simple. As a prerequisite, the project has to be built with the above
commands already.

The below commands require at least CMake 3.15 to run, because that is the
version in which [Install a Project][2] was added.

Here is the command for installing the release mode artifacts with a
single-configuration generator, like the Unix Makefiles one:

```sh
cmake --install build
```

Here is the command for installing the release mode artifacts with a
multi-configuration generator, like the Visual Studio ones:

```sh
cmake --install build --config Release
```

[1]: https://cmake.org/download/
[2]: https://cmake.org/cmake/help/latest/manual/cmake.1.html#install-a-project
