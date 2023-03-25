# simpact_vst

## External installation dependencies
* CMake
 * add to path
* libtorch
  * downlaod from pytorch.org 
  * Stable(2.0.0)-Windows-LibTorch-C++/Java/CPU (Debug version)
  * extract libtorch to a known directory
* JUCE
  * added as a submodule after cloning

## Installation
1. Clone this repository 
`cd /your/local/install/folder` `git clone https://github.com/Ann-yang00/simpact_vst.git` 
2. Install the dependencies mentioned above
3. Add JUCE as a submodule to the root directory of this repo using 
`git submodule init` `git submodule update`

### Build
1. In the cloned directory
2. `mkdir build; cd build`
3. `cmake -DCMAKE_PREFIX_PATH="your/absolute/path/to/libtorch" ..`
4. `cmake --build . --config Debug` 
7. Copy DLL files in `simpact_vst\build\AudioPluginExample_artefacts\Debug` to your executable directory (e.g. C:\Program Files\REAPER (x64))
