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
git clone --recursive https://github.com/IENT/PyCabac.git
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

In order to be able to run the unit tests, `numpy` is required:

```bash
pip install numpy
```

## Demo

We expect that you are familiar with the concept of binary arithmetic coding.
Hence, from a user perspective, it is most probably sufficient to understand how you can encode a binary sequence with PyCabac.
For demos, check the Python scripts in either the `demo` or the `tests` folder:

* `demo/tracing.py` demonstrates how to trace the encoding process of a binary sequence. Both the MPS and the probability of the current context can be extracted for each context.
* `tests/test_cabac.py` shows how to encode integer symbols given a specific binarization or encode binary sequences directly.
* `tests/test_cabac_timing.py` shows different ways of encoding an integer sequence and measures encoding/decoding time for comparison.
* `tests/test_cabac_sequence.py` shows how to encode an integer sequence given a specific binarization and a predefined context set.

For a more technical/scientific description of the probability modelling in CABAC, you may want to have a look at:

```bibtex
H. Schwarz et al., "Quantization and Entropy Coding in the Versatile Video Coding (VVC) Standard," in IEEE Transactions on Circuits and Systems for Video Technology, doi: 10.1109/TCSVT.2021.3072202.
```

## Pybind11 Example

```python
import pybind11_example
pybind11_example.add(1, 2)
```

## Python Unit Tests

Python unit tests are done using Python's built-in ``unittest`` module. Note that ``numpy`` is required. Python unit tests live in the ``tests`` folder. As long as all of your files match ``'test*.py'``, ``unittest`` will automatically discover them. Run ``python3 -m unittest discover [--verbose]`` to run all unit tests.

## C++ Unit Tests

C++ unit tests are done using Catch2 (https://github.com/catchorg/Catch2). C++ unit tests live in the ``tests`` folder. The tests executable can be built with CMake. On Linux/macOS use the following commands to build the tests executable:

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make --jobs
```

This will leave the tests executable under ``build/tests/tests``.

## Citation

If you use this software, please cite it as below.

```
@inproceedings{RoMeScVo23,
  author = {Rohlfing, Christian and Meyer, Thibaut and Schneider, Jens and Voges, Jan},
  title = {Python Wrapper for Context-based Adaptive Binary Arithmetic Coding},
  booktitle = {Visual Communications and Image Processing {VCIP}~'23},
  address = {Jeju, South Korea},
  year = {2023},
  month = dec,
  doi = {10.1109/VCIP59821.2023.10402639},
  publisher = {{IEEE}, Piscataway}
}
```

## Who do I talk to?

Jan Voges <[voges@tnt.uni-hannover.de](mailto:voges@tnt.uni-hannover.de)>

Christian Rohlfing <[rohlfing@ient.rwth-aachen.de](mailto:rohlfing@ient.rwth-aachen.de)>

Jens Schneider <[schneider@ient.rwth-aachen.de](mailto:schneider@ient.rwth-aachen.de)>
