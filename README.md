# IOTA client to communicate with the REST API of the nodes 

[TOC]

This repo implements  a client to communicate with the IOTA nodes using the  [Core REST API](https://github.com/iotaledger/tips/blob/main/tips/TIP-0025/tip-0025.md) and the [UTXO Indexer REST API](https://github.com/iotaledger/tips/blob/main/tips/TIP-0026/tip-0026.md).


## Installing the library 

### From source code
```
git clone https://github.com/EddyTheCo/Qclient-IOTA.git 

mkdir build
cd build
qt-cmake -G Ninja -DCMAKE_INSTALL_PREFIX=installDir -DUSE_THREADS=ON -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DBUILD_DOCS=OFF ../Qclient-IOTA

cmake --build . 

cmake --install . 
```
where `installDir` is the installation path.
One can choose to build or not the tests and the documentation with the `BUILD_TESTING` and `BUILD_DOCS` variables.
The use or not of multithreading is set by the `USE_THREADS` variable.

### From GitHub releases
Download the releases from this repo. 

## Adding the libraries to your CMake project 

```CMake
include(FetchContent)
FetchContent_Declare(
	IotaClient	
	GIT_REPOSITORY https://github.com/EddyTheCo/Qclient-IOTA.git
	GIT_TAG vMAJOR.MINOR.PATCH 
	FIND_PACKAGE_ARGS MAJOR.MINOR CONFIG  
	)
FetchContent_MakeAvailable(IotaClient)
target_link_libraries(<target> <PRIVATE|PUBLIC|INTERFACE> IotaClient::qclient)
```


## API reference

You can read the [API reference](https://eddytheco.github.io/Qclient-IOTA/) here, or generate it yourself like
```
cmake -DBUILD_DOCS=ON ../
cmake --build . --target doxygen_docs
```

## Contributing

We appreciate any contribution!


You can open an issue or request a feature.
You can open a PR to the `develop` branch and the CI/CD will take care of the rest.
Make sure to acknowledge your work, and ideas when contributing.

