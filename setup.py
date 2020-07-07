#
# Copyright (C) 2020 GrammaTech, Inc.
#
# This code is licensed under the MIT license. See the LICENSE file in
# the project root for license terms.
#
# This project is sponsored by the Office of Naval Research, One Liberty
# Center, 875 N. Randolph Street, Arlington, VA 22203 under contract #
# N68335-17-C-0700.  The content of the information does not necessarily
# reflect the position or policy of the Government and no official
# endorsement should be inferred.
#
import imp
import unittest
import setuptools


__version__ = imp.load_source(
    "pkginfo.version", "gtirb_functions/version.py"
).__version__


def gtirb_functions_test_suite():
    test_loader = unittest.TestLoader()
    test_suite = test_loader.discover("tests", pattern="test_*.py")
    return test_suite


if __name__ == "__main__":
    with open("README.md", "r") as fh:
        long_description = fh.read()

    setuptools.setup(
        name="gtirb-functions",
        version=__version__,
        author="Grammatech",
        author_email="gtirb@grammatech.com",
        description="Utilities for dealing with functions in GTIRB",
        packages=setuptools.find_packages(),
        test_suite="setup.gtirb_functions_test_suite",
        install_requires=["gtirb", "gtirb-capstone"],
        classifiers=["Programming Language :: Python :: 3"],
        entry_points={
            "console_scripts": [
                "gtirb-functions=gtirb_functions.__main__:main"
            ],
        },
        long_description=long_description,
        long_description_content_type="text/markdown",
        url="https://github.com/grammatech/gtirb-functions",
        license="MIT",
    )
