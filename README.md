# DCScan-ModulesAPI
A  set of libraries used to implement functionalities over a Double Crystal Spectrometer acquisition system.
You can view the GUI source code (which implements these binaries) at the [DCScan-GUI](http://github.com/lPrimemaster/DCScan-GUI) repository.


## Documentation
#### Locally
Documentation for the DCScan API is available by downloading and building this project (see [Building](#building)). The docs should be automatically generated using Doxygen in the chosen build folder in html and LaTeX formats.
#### Online
It will also be available in a github.io, automatically deployed using some sort of continuous integration (see [this](https://gist.github.com/vidavidorra/548ffbcdae99d752da02)).

## Building

> :warning: DCScan-ModulesAPI is only available for windows, since a few functionalities are platform dependent. **Compiling for any other operating system will fail.**

### Simple Mode
To build DCScan-ModulesAPI is as simple as running the `build.cmd` from console with one of the following arguments:
| Argument  |                            Description                           |
|-----------|------------------------------------------------------------------|
| ALL_BUILD | Builds all the libraries as dll's inside a folder named `build`. If not configured, runs the CMake generator for Visual Studio. |
| CLEAN     | Deletes all build files.
| RUN_TESTS | Builds the test package and runs all tests for the API.          |

##### Example
This will build all the libraries and run their respective tests:
```shell
cd repo_location
build ALL_BUILD
build RUN_TESTS
```
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
