# Distributed Multi-Agent Stock Exchange Environment

 Multi-agent distributed environment to model the global network of contemporary financial exchanges. The modular and highly configurable environment allows to setup and deploy stock exchange agents and trader agents in different geographical locations using commercial cloud-computing services. 

 ## Table of Contents
1. [Getting Started](#getting-started)
   * [Pre-requisites](#pre-requisites)
   * [Building](#building)
2. [Usage](#usage)
   * [Configuration](#configuration)
   * [Running simulations](#running)
3. [Project Status](#project-status)
4. [Acknowledgements](#acknowledgements)
5. [License](#license)

## Getting Started

### Pre-requisites
* [CMake](https://cmake.org/) 3.22+
* [Boost C++ library](https://www.boost.org/) 1.76+
* [Docker](https://docs.docker.com/get-docker/) (Optional)

### Building
To build locally:
```bash
cmake . -B build/ 
cmake --build build
```

## Usage

### Configuration
To configure the simulation, create the XML configuration file according to the specification. See [simulationexample.xml](./simulationexample.xml) for reference.

### Running simulations

To run a simulation node <br>
`./simulation node --port <port>`

To run the simulation orchestrator <br>
`./simulation orchestrator --port <port> --config <path-fo-config-file>`

### Project Status
The project is currently in active development and the implementation is subject to change.

### Acknowledgements
This project uses the following open-source libraries:
* [Boost C++ library](https://www.boost.org/)
* [pugixml C++ library](https://github.com/zeux/pugixml)
   
### License
The project is open-source under the terms of the [MIT License](./LICENSE).