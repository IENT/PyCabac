# PYCABAC

[Easy|Educational|Extensible|Experimental] CABAC

A Python port to the VTM CABAC engine

---

## Prerequisites

### Linux, macOS

* Compiler with C++11 support
* CMake >= 2.8.12

### Windows

* Visual Studio 2015
* CMake >= 3.1

## Installation

Just clone this repository and install via pip. Note the `--recursive` option which is
needed for the pybind11 submodule:

```bash
git clone --recursive git@github.com:IENT/PyCabac.git
pip install .
```

With the `setup.py` file included in this example, the `pip install .` command will
invoke CMake and build the pybind11 module as specified in `CMakeLists.txt`.

Note: We recommend the usage of a virtual environment:

```bash
python3 -m venv venv
source venv/bin/activate
```

Note: Uninstall via:

```bash
pip uninstall pycabac
```

## Demo

We expect that you are familiar with the concept of binary arithmetic coding.
Hence, from a user perspective, it is most probably sufficient to understand how you can encode a binary sequence with PyCabac.
For a demo, check the Python script in the demo folder.
For a more technical/scientific description of the probality modelling in CABAC, you may want to have a look at:
```
H. Schwarz et al., "Quantization and Entropy Coding in the Versatile Video Coding (VVC) Standard," in IEEE Transactions on Circuits and Systems for Video Technology, doi: 10.1109/TCSVT.2021.3072202.
```


## Pybind11 Example

```python
import pybind11_example
pybind11_example.add(1, 2)
```

## Python Unit Tests

Python unit tests are done using Python's built-in ``unittest`` module. Python unit tests live in the ``tests`` folder. As long as all of your files match ``'test*.py'``, ``unittest`` will automatically discover them. Run ``python3 -m unittest discover [--verbose]`` to run all unit tests.

## C++ Unit Tests

C++ unit tests are done using Catch2 (https://github.com/catchorg/Catch2). C++ unit tests live in the ``tests`` folder. The tests executable can be built with CMake. On Linux/macOS use the following commands to build the tests executable:

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make --jobs
```

This will leave the tests executable under ``build/tests/tests``.

## Who do I talk to?

Jan Voges <[voges@tnt.uni-hannover.de](mailto:voges@tnt.uni-hannover.de)>

Christian Rohlfing <[rohlfing@ient.rwth-aachen.de](mailto:rohlfing@ient.rwth-aachen.de)>

Jens Schneider <[schneider@ient.rwth-aachen.de](mailto:schneider@ient.rwth-aachen.de)>
