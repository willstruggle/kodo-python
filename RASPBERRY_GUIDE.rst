Compiling on Raspberry Pi
=========================

Before you start, make sure you have the following at your ready:

* A valid license for the Kodo library.
* Fully functioning Raspberry Pi with Raspbian 9 (Strecth) installed.
* Some way to interact with your Raspberry Pi (keyboard or SSH access).
* An Internet Connection for your Raspberry Pi.
* ~30 minutes worth of coffee or similar beverage (~1.5 hours for Raspberry Pi 1).

Start by booting up the Raspberry Pi and open up a terminal - if you have a
headless install, this should be fairly trivial.

First, update your package manager::

    sudo apt-get update

You are now ready to install the required packages::

    sudo apt-get install git-core build-essential python-dev g++

Also make sure that ``libpython`` is installed (this might be done automatically when 
``python-dev`` is installed). On my Raspberry Pi 1, the package was called `libpython2.7`,
but on my Raspberry Pi 2 it was called `libpython-dev`. This will depend on your
distribution.

We need to make sure that the default compiler is g++ 6.3 (or later) on the Raspberry Pi.
You can quickly check the default version like this::

    g++ --version

If you see an older version, please remove the corresponding packages or change the
default using update-alternatives.

You have configured the Raspberry Pi so that it's ready for compiling kodo-python.
Clone the repository::

    git clone https://github.com/steinwurf/kodo-python.git

Change directory to the repository::

    cd kodo-python

Run the waf configure step - in this process you will be queried for your
Github username and password multiple times::

    python waf configure

When the configuration has finished successfully, you can run the build command
(note that ``-j1`` forces waf to use a single CPU core, otherwise you will surely
exhaust the available RAM)::

    python waf build -j1
    
If the build step fails with an error like this, then you need to add more swap space
following the instructions below::
    
    virtual memory exhausted: Cannot allocate memory

The build step takes about 30 minutes on RPi 2 and 3 (but it could be a few hours on RPi 1).

.. image:: https://imgs.xkcd.com/comics/compiling.png
    :target: https://xkcd.com/303/
    :alt: Compiling

Finally enjoy your freshly made kodo-python for the Raspberry Pi!!!

Increasing swapfile size
........................
Because the compilation of kodo-python is rather memory-intensive, the installed
memory and swapfile are not sufficient (the Pi 1 will run out of memory).

The default swapfile size on Raspbian is only 100 MB, but you can easily
increase this size following this guide:
https://www.bitpi.co/2015/02/11/how-to-change-raspberry-pis-swapfile-size-on-rasbian/

Edit the following file::

    sudo nano /etc/dphys-swapfile
    
Increase the swapfile size to 1024 MB::
    
    CONF_SWAPSIZE=1024

Restart the dphys-swapfile service::

    sudo /etc/init.d/dphys-swapfile restart

Check that the swapfile was correctly resized::

    cat /proc/swaps

The command should output something like this::

    Filename      Type          Size  Used  Priority
    /var/swap     file       1048572     0        -2
