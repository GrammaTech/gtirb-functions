from conans import ConanFile, CMake
from conans.errors import ConanInvalidConfiguration
import os
import re

main_branch = "master"


def get_version():
    if "CI_COMMIT_REF_NAME" in os.environ:
        branch = os.environ["CI_COMMIT_REF_NAME"]
        if branch == main_branch:
            return "dev"
    try:
        with open("version.txt") as f:
            s = f.read()
            match = re.search(
                r"VERSION_MAJOR(\s+)(\S+)(\s+)"
                r"VERSION_MINOR(\s+)(\S+)(\s+)"
                r"VERSION_PATCH(\s+)(\S+)(\s+)",
                s,
            )
            if match:
                major = match.group(2)
                minor = match.group(5)
                patch = match.group(8)
                return major + "." + minor + "." + patch
            else:
                return "<ERROR: no version found>"
    except Exception:
        return None


def branch_to_channel(branch):
    if re.match(r"^release-.*", branch):
        return "stable"
    else:
        return branch.replace("/", "+")


class Properties:
    name = "gtirb-functions"
    version = get_version()
    rel_url = "rewriting/gtirb-functions"
    exports_sources = "*"

    @property
    def description(self):
        return "%s library" % self.name

    @property
    def url(self):
        return "https://git.grammatech.com/%s" % self.rel_url

    @property
    def conan_channel(self):
        channel = "local"
        if "CI_COMMIT_REF_NAME" in os.environ:
            branch = os.environ["CI_COMMIT_REF_NAME"]
            channel = branch_to_channel(branch)
        return channel

    @property
    def archived_channels(self):
        # Add to this list branch names to have conan packages for
        # branches archived in gitlab.
        archived_branches = [main_branch]
        # Also, archive the 'stable' channel, where all stable versions
        # will be uploaded
        archived_channels = ["stable"]
        return archived_channels + list(
            map(branch_to_channel, archived_branches)
        )

    @property
    def conan_ref(self):
        return "%s/%s" % (self.rel_url.replace("/", "+"), self.conan_channel)

    @property
    def conan_recipe(self):
        return "%s/%s@%s" % (self.name, self.version, self.conan_ref)


class GtirbFunctionsConan(Properties, ConanFile):
    author = "GrammaTech Inc."
    generators = "cmake"
    settings = ("os", "compiler", "build_type", "arch")
    license = "MIT"

    boost_version = "1.69.0"
    gtirb_version = "dev"
    requires = (
        "gtirb/%s@rewriting+gtirb/%s"
        % (gtirb_version, "master" if gtirb_version == "dev" else "stable"),
        "boost/%s" % (boost_version),
    )

    def configure(self):
        if (
            self.settings.compiler == "gcc"
            and self.settings.compiler.libcxx != "libstdc++11"
        ):
            raise ConanInvalidConfiguration(
                "%s requires libstdc++11 ABI, update your conan profile"
                % self.name
            )

    def build(self):
        cmake = CMake(self)
        defs = {
            "CMAKE_VERBOSE_MAKEFILE:BOOL": "ON",
            "ENABLE_CONAN:BOOL": "ON",
        }
        cmake.configure(source_folder="./", defs=defs)
        cmake.build()
        cmake.test(output_on_failure=True)
        cmake.install()
        # The patch_config_paths() function will change absolute paths in the
        # exported cmake config files to use the appropriate conan variables
        # instead.
        # It is an experimental feature of conan, however, so if you're having
        # trouble with paths in the cmake of the conan package, it could that
        # this function is no longer doing what we want.
        cmake.patch_config_paths()

    def package(self):
        self.copy("*.h", dst="include", src=self.name)
        self.copy("*%s.lib" % self.name, dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = [self.name]
