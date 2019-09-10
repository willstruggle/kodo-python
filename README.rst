License
-------

A valid Kodo license is required if you wish to use this project.

Please request a license by **filling out the license request** form_.

Kodo is available under a research- and education-friendly license,
you can see the details here_.

If you try to configure without a valid license, then you will get an error!

.. _form: http://steinwurf.com/license/
.. _here: http://steinwurf.com/research-license/

About
-----

kodo-python contains a set of high-level Python bindings for the Kodo Network
Coding C++ library. The bindings provide access to basic functionality provided
by Kodo, such as encoding and decoding data. The examples folder contains
sample applications showing the usage of the Python API.

.. image:: http://buildbot.steinwurf.dk/svgstatus?project=kodo-python
    :target: http://buildbot.steinwurf.dk/stats?projects=kodo-python
    :alt: Buildbot status

If you have any questions or suggestions about this library, please contact
us at our developer mailing list (hosted at Google Groups):

* http://groups.google.com/group/steinwurf-dev

.. contents:: Table of Contents:
   :local:

Requirements
------------

First of all, follow `this Getting Started guide
<http://docs.steinwurf.com/getting_started.html>`_ to install
the basic tools required for the compilation (C++14 compiler, Git, Python).

The compilers used by Steinwurf are listed at the bottom of the
`buildbot page <http://buildbot.steinwurf.com>`_.

Linux
.....

These steps may not work with your specific Linux distribution, but they may
guide you in the right direction.

First, acquire the required packages from your package management system::

  sudo apt-get update
  sudo apt-get install python build-essential libpython-dev python-dev

If you are using Python 3, you'll need to install ``libpython3-dev`` instead.

Mac OSX
.......

Install the latest Command Line Tools and/or XCode to compile this project.
The latest tested version is 10.2.1, so please upgrade if your CLT or XCode
is older than that.

Python 2.7.10 is pre-installed on OSX, but some required Python headers are
missing, so you need to install a more recent Python version from Homebrew
(or Macports).

You can choose to install Homebrew's Python 2 which will become the default
Python on your system, so you can call waf using the ``python`` command::

    brew install python@2
    python waf configure
    python waf build

You can also choose Homebrew's Python 3, but then you must always use the
``python3`` command for invoking waf::

    brew install python3
    python3 waf configure
    python3 waf build

Windows
.......

First of all, you need to install Visual Studio 2017. There are many variants,
but you should basically get the same C++14 compiler: VS Express, Community,
Professional, Enterprise or the standalone Build Tools might all work.

You can choose to install Python 2.7 or Python 3.7+. It is very important to
install **32-bit Python** for a 32-bit VS toolchain and you need
**64-bit Python** for a 64-bit VS toolchain. Some Visual Studio versions only
provide a 32-bit toolchain, so this might be the only option.

If you installed Python 2.7, then you need to set the ``VS90COMNTOOLS``
environment variable to point to the folder that actually contains
``vcvarsall.bat`` (this depends on the version of VS2017 that you installed)::

    C:\Program Files (x86)\Microsoft Visual Studio\2017\WDExpress\VC\Auxiliary\Build
    C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build
    C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build
    C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build

If you installed Python 3.7+, then you need to set the ``VS140COMNTOOLS``
environment variable to point to the folder that actually contains
``vcvarsall.bat`` (this depends on the version of VS2017 that you installed)::

    C:\Program Files (x86)\Microsoft Visual Studio\2017\WDExpress\VC\Auxiliary\Build
    C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build
    C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build
    C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build

After setting the appropriate environment variable, you need to make sure that
Python's distutils can find a valid location for ``vcvarsall.bat``, because
waf will call that batch file to obtain some compiler flags. If you can execute
the following test script without getting an exception, then you should be able
to configure kodo-python using waf::

    if __name__ == "__main__":
        from distutils import log
        log.set_threshold(log.DEBUG)
        from distutils.msvccompiler import MSVCCompiler
        dist_compiler = MSVCCompiler()
        dist_compiler.initialize()
        print("Compile options:")
        print(dist_compiler.compile_options)
        print("LDFLAGS:")
        print(dist_compiler.ldflags_shared)

If you have any issues with the test script, then most likely you need to
apply this one-liner patch to ``msvc9compiler.py`` in ``Python27\Lib\distutils``
or ``Python37\Lib\distutils``: https://bugs.python.org/file45916/vsforpython.diff

The problem is that the location of ``vcvarsall.bat`` has changed with
respect to the Common Tools folder, so distutils cannot find it without
the patch. The issue is explained here: https://bugs.python.org/issue23246


Building From Source
--------------------

You need to build the Python bindings from source.

First, clone the project::

  git clone git@github.com:steinwurf/kodo-python.git

Configure and build the project::

  cd kodo-python
  python waf configure
  python waf build

After building the project, you should find the resulting ``kodo.so``,
``kodo.dylib`` or ``kodo.pyd`` file here (the actual path and extension
depend on your OS)::

  build/linux/kodo.so
  build/darwin/kodo.dylib
  build/win32/kodo.pyd

You can copy this file to the same folder as your Python scripts, or you
can copy it to your PYTHONPATH (so that you can import it from anywhere).

Then you can import the module in your Python script::

  >>> import kodo

Special Options
...............

With the ``enable_codecs`` option, you can configure kodo-python to only enable
some desired codecs and disable all others. For example::

    python waf configure --enable_codecs=rlnc

Run ``python waf --help`` to list the available codecs. You can even
select multiple codecs with a comma-separated list::

    python waf configure --enable_codecs=rlnc,fulcrum

Compilation Issues on Linux
...........................

The compilation process might take a long time on certain Linux systems if
less than 4 GB RAM is available. The g++ optimizer might consume a lot of RAM
during the compilation, so if you see that all your RAM is used up, then
you can try to constrain the number of parallel jobs to only one during the
build step::

    python waf build -j 1

With this change, a fast compilation is possible with only 2 GB RAM.

This issue is specific to g++ (which is the default compiler on Linux), and
the RAM usage and the compilation time can be much better with clang.
The code produced by clang is also fast.

If the compilation does not work with g++, then you can install clang like
this (on Ubuntu and Debian)::

    sudo apt-get install clang

Then you can configure the project to use clang++::

    CXX=clang++ python waf configure

Compiling on the Raspberry Pi
.............................

The detailed instructions for compiling the project on the Raspberry Pi
are found in our `Raspberry guide
<https://github.com/steinwurf/kodo-python/blob/master/RASPBERRY_GUIDE.rst>`_.
