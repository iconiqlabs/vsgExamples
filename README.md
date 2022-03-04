# vsgExamples
Example programs that test and illustrate how to use the [VulkanSceneGraph](https://github.com/vsg-dev/VulkanSceneGraphPrototype/) and optional add-on libraries. The example programs are grouped according to the type of features they illustrate their dependencies.

* [core](src/core/) examples
* [maths](src/maths/) examples
* [viewer](src/viewer/) examples
* [vk](src/vk/) examples
* [text](src/text/) examples
* [rtx](src/rtx/) examples
* [state](src/state/) examples
* [commands](src/commands/) examples
* [introspection](src/introspection/) examples
* [io](src/io/) examples
* [ui](src/ui/) examples
* [traversals](src/traversals/) examples
* [platform](src/platform/) examples
* [utils](src/utils/) examples

## Quick Guide to Building the vsgExamples

### Prerequisites:
* C++17 compliant compiler i.e.. g++ 7.3 or later, Clang 6.0 or later, Visual Studio 2017 or later.
* [Vulkan](https://vulkan.lunarg.com/) 1.1 or later.
* [CMake](https://www.cmake.org) 3.7 or later.
* [VulkanSceneGraph](https://github.com/vsg-dev/VulkanSceneGraph/) use master
* [vsgXchange](https://github.com/vsg-dev/vsgXchange/) optional - used for 3d model and image loaders
* [vsgImGui](https://github.com/vsg-dev/vsgimGui/) optional - used for in window GUI examples

The above dependency versions are known to work so they've been set as the current minimum, it may be possible to build against older versions.  If you find success with older versions let us know and we can update the version info.

### Command line build instructions:
To build and install in source, with all dependencies installed in standard system directories:

    git clone https://github.com/vsg-dev/vsgExamples.git
    cd vsgExamples
    cmake .
    make -j 8

Full details on how to build of the VSG can be found in the [INSTALL.md](INSTALL.md) file.

## Running examples

After you have built the examples you should set your binary search path to the vsgExamples/bin directory, and the VSG_FILE_PATH env vars.

	export PATH="$PATH:/path/to/vsgExamples/bin"
	export VSG_FILE_PATH=/path/to/vsgExamples/data

Then run examples:

	vsgmaths # run simple tests of vsg/maths functionality
	vsgdraw # run the vsgdraw example (a port of VulkanTutorial)
