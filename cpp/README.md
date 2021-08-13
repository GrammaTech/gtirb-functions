[![pipeline status](https://git.grammatech.com/research/templates/CPP/badges/master/pipeline.svg)](https://git.grammatech.com/research/templates/CPP)
[![coverage report](https://git.grammatech.com/research/templates/CPP/badges/master/coverage.svg)](https://git.grammatech.com/research/templates/CPP)

# C++ Template #

This is a template for a pure-C++ project that uses cpack to generate
debian and rpm installers.  The CI is configured to use a kubernetes runner to
build and test the project, as well as build, test, and upload the
installers for Ubuntu 20 and CentOS 8.

## Important

Do not hardcode any GrammaTech internal URLs into our repositories, since
they may leak out to externally-visible repositories.  This README has some
references for documentation reasons; when copying this template to a new
repository, this README should be flushed and replaced with a fresh document.

## Conan initial configuration

Note: you can use docker image with pre-configured Conan to build this template project locally: ```docker run --rm -it -v $(pwd):/cpp -w /cpp $DOCKER_REGISTRY/research/templates/cpp/ubuntu20```.

This template demonstrates how to use GitLab Conan integration to track dependencies on other C++ projects. It also creates its own Conan package on GtiLab CI. You need to add a remote for our GitLab instance in order to use our own Connan packages:
```shell
conan remote add gitlab https://git.grammatech.com/api/v4/packages/conan
```

You can also set up your credentials for `gitlab` remote to avoid being asked every time:
```shell
conan user <user> -r gitlab -p <GitLab access token>
```

Linux only: make sure you have the same `settings.compiler.libcxx` setting we use for other Conan packages:
```shell
  conan profile new default --detect
  conan profile update settings.compiler.libcxx=libstdc++11 default
```

## Building

```
mkdir build && cd build
conan install ..
cmake ..
cmake --build .
```

You can also run `create-conan-package.py` to create the Conan package locally (by default this scrtipt won't try to upload anything).

## Running tests

In the build directory:

```
ctest
```

## Specifying dependencies on other GitLab Conan packages

This project depends on "ref_ptr/0.1@gt+utils+ref_ptr/stable" package. When you don't need your own Conan package for your project and just need to inject dependency on another C++ project on GitLab:

1. Create `conanfile.txt` with required dependencies specified, e.g.:
    ```
    [requires]
    ref_ptr/0.1@gt+utils+ref_ptr/stable

    [generators]
    cmake
    ```

1. Add these lines to your `CMakeLists.txt`:
    ```
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
    ```

1. Use "conan install" to generate `conanbuildinfo.cmake` you included in the previous step
    ```shell
     mkdir build && cd build
     conan install ..
     ```

1. Build your project with cmake as usual:
    ```shell
    cmake ..
    cmake --build .
    ```

## Creating your own Conan package and deploying it to GitLab

- Create a new package recipe template
    ```shell
    conan new hello/0.1
    ```
This will create the default `conanfile.py`. Edit it accordingly and make sure to specify dependencies (e.g. `requires = "ref_ptr/0.1@gt+utils+ref_ptr/stable"`). You don't need `conanfile.txt` from the previous topic if you have `conanfile.py`. Look for `build` job in `.gitlab-ci.yml` - this job creates a package and uploads it to GitLab.


# Building the installers

The project builds 3 installers: one for the shared library, one for the command line tool, and a dev package which includes the header files with the library:

```
cpack -G DEB -D CPACK_HELLO_PACKAGE=lib
cpack -G DEB -D CPACK_HELLO_PACKAGE=dev
cpack -G DEB -D CPACK_HELLO_PACKAGE=driver
```

These commands will build debian packages. Specify "-G RPM" to build rpm packages.


# Setting up your CI

You will need to go into your project's settings page in GitLab and change a few
settings:

* Assign a runner to this project. This runner must have Kubernetes enabled.

# Checklist

Here's how you change this template into something of your own:

- [ ] The name of the example project is 'hello', so there are many references to this string (lowercase, Capitalized, and ALLCAPS) in various places throughout the template.  You should find and replace this string with the name of your project instead:
    - [ ] Replace references to 'hello' with your project name: `find ./ -type f \( -iname '*.cpp' -o -iname '*.hpp' -o -iname '*.h' -o -iname '*.txt' -o -iname '*.in*' -o -iname '*.cmake' -o -iname '*.yml' \) -exec sed -i -e 's/hello/[your project name]/g;s/HELLO/[your project name in all caps]/g;s/Hello/[your project name capitalized]' \{\} \;`
    - [ ] Rename the include directory: `mv include/hello include/[your project name]`
    - [ ] Rename the config file: `mv helloConfig.cmake.in [your project name]Config.cmake.in`
    - [ ] In `conanfile.py`, change `Properties.name`, `Properties.rel_url`, and `Properties.description`.
    - [ ] Also in `conanfile.py`, change the `HelloConan` class name and the `requires` property under.
- [ ] Does your project need to generate debian/rpm packages?  If not, you should remove the portions of the repository which do that:
    - [ ] In `.gitlab-ci.yml`, remove `cpack-*` stages and `(debian|rpm)-*` jobs (everything below the `# CPack jobs:` comment).
    - [ ] In `CMakeLists.txt`, remove the `Debian/RPM package generation` section. Remove `cpack-config.cmake` file.
- [ ] If your project does need to generate debian/rpm packages without executables then remove `*driver*` jobs/commands from `.gitlab-ci.yml` and corresponding `driver*` entries from `cpack-config.cmake`. Remove `*dev*` entries if your project does not create header files.
- [ ] This project has a sample dependency on `ref_ptr`.  Search for instances of that string as a model for how to add your own dependences (and remove the dependence on `ref_ptr` if not needed).
- [ ] Add your source files to the `src` directory, and add your header files to the `include` directory.
- [ ] Replace the references to the old source/header files in `src/CMakeLists.txt` with your source/header files.
- [ ] Replace the source files under `src/test` with your own tests, and replace the references to the old tests in `src/test/CMakeLists.txt`
- [ ] Set the `DOCKER_REGISTRY` variable to `docker.grammatech.com` in the Gitlab UI under repo settings > CI/CD > Variables.  (This is so we don't leak the URL to the public)
- [ ] In .gitlab-ci.yaml, the default Docker `image` in this template points to a ubuntu20 within this template's project namespace.  You can continue to use this, or push one to your project's namespace.  For the latter, go to your project in GitLab, Packages & Registries, Container Registry, and follow the directions there, which should look like:
    - [ ] docker login docker.grammatech.com
    - [ ] docker build [-f Dockerfile.ubuntu20] -t docker.grammatech.com/your-project-path/hello .
    - [ ] docker push docker.grammatech.com/your-project-path/hello
- [ ] If you are generating debian/rpm packages, remember to change the `CPACK_PACKAGE_CONTACT` and `CPACK_PACKAGE_DESCRIPTION_SUMMARY` variables in `CMakeLists.txt`.
- [ ] Finally, add the appropriate license file and uncomment `CPACK_PACKAGE_RESOURCE_FILE_LICENSE` variable in `CMakeLists.txt`.
