# DCScan-ModulesAPI
A  set of libraries used to implement functionalities over a Double Crystal Spectrometer acquisition system.
You can view the GUI source code (which implements these binaries) at the [DCScan-GUI](http://github.com/lPrimemaster/DCScan-GUI) repository.

![TCI](https://travis-ci.org/lPrimemaster/DCScan-ModulesAPI.svg?branch=master)

## Documentation
#### Locally
Documentation for the DCScan API is automatically generated and made available by downloading and building this project (see [Building](#building)). The docs are generated using Doxygen in the chosen build folder in html and LaTeX formats.
> :warning: To successfully generate the documentation automatically you should have Doxygen installed when building.
#### Online
It is also available [here](https://lprimemaster.github.io/DCScan-ModulesAPI/), automatically deployed using travis-CI. Note that this documentation does not include internal documentation (see `INTERNAL_DOCS` [here](#available-cmake-options)).

## Building

> :warning: DCScan-ModulesAPI is only available for windows, since a few functionalities are platform dependent. **Compiling for any other operating system will most likely fail.**

### Simple Mode
To build DCScan-ModulesAPI is as simple as running the `build.cmd` from console with one of the following arguments:
| Argument  |                            Description                           |
|-----------|------------------------------------------------------------------|
| ALL_BUILD | Builds all the libraries as dll's inside a folder named `build`. If not configured, runs the CMake generator for Visual Studio. This command supports a second argument that specifies cmake's aditional parameters, like options. See [example](#example) for more details. |
| INSTALL   | Installs the DCScan-ModulesAPI compiled dependencies on a default directory and registers their location in windows registry, so CMake can locate the package with `find_package()` in config mode (see more [here](https://cmake.org/cmake/help/latest/command/find_package.html)). This is particularly useful if compiling another C++ binary with CMake (see [Using the libraries](#using-the-libraries)). |
| CLEAN     | Deletes all build files. |
| RUN_TESTS | Builds the test package and runs all tests for the API.          |
| CTARGETS  | Builds teh custom targets only. These targets consist in additional binaries that provide equipment testing. |

##### Available cmake options

|   Option    | Default | Description |
|-------------|---------|-------------|
| BUILD_TESTS |   OFF   | Enables building tests in the build stage. **DO NOT** build the release version with this option ON, it will expose unecessary internal functions to the API. |
| INTERNAL_DOCS | OFF   | Enables internal documentation of the API. This is specially usefull if you wish to fiddle with the internal code for further implementation. Note that this documentation might be not as detailed as the one available with the exposed API. |
| CI_DOC_GENERATE | OFF | Enables building only the docs, skipping the build step and cmake's source configuration. Intended to use with CI for auto doc generation. Can be used to generated only documentation if needed. |

Separate options using a spaces. For example:
```shell
build ALL_BUILD "-DBUILD_TESTS=OFF -DINTERNAL_DOCS=ON ..."
```


##### Example
This will build all the libraries and run their respective tests:
```shell
# Clone the repo
git clone https://github.com/lPrimemaster/DCScan-ModulesAPI.git
cd DCScan-ModulesAPI

# Build the libraries
build ALL_BUILD "-DBUILD_TESTS=ON" && build RUN_TESTS
```
> :warning: **DO NOT** build the release version with "-DBUILD_TESTS=ON" (see [here](#available-cmake-options)). Use it only for testing.


### Advanced Mode
A more powerful way to build DCScan-ModulesAPI is by running CMake on your own, using a custom generator (like `ninja` for example). These features should work, although untested. Use at own risk.
In the main directory of the project run:
```shell
cmake CMakeLists.txt
```
After this you can compile the binaries with whatever project CMake generated for you.

## Using the libraries
If you're compiling these library on their own, chances are you'd like to use them wherever to interact with the system trough a third party script or binary. There multiple options available are displayed in detail below.

(WIP: TODO)
