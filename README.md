# simpact_vst

## External installation dependencies

- cmake
- libtorch
- JUCE

## Installation

1. Clone this repository
2. Install the dependencies mentioned above
3. Add cmake to your path
4. Add JUCE as a submodule to the root directory of this repo
5. Extract libtorch to a known directory

### In the command prompt

1. Navigate to the cloned repo directory
2. `mkdir build`
3. `cmake -DCMAKE_PREFIX_PATH="/absolute/path/to/libtorch" ..`
4. `cmake --build . --config Debug` (make sure this corresponds to the libtorch download version)
5. Change the cmake libtorch directory
6. `git submodule add JUCE`
7. Copy DLL files to the executable directory
