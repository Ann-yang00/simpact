# simpact_vst
<img src=/images/plugin_screensnip.PNG width=60% height=60%>

### Summary
Simpact (Simulated Impact) is a plugin that generates impact foley sounds with minimal, intuitive controls based on and trained using [RAVE][rave_repo]. MIDI note ONs trigger decoded audio playback, allowing users to place notes corresponding to visual cues. Imported audio files are mapped into the latent space, whose perceptual quality can then be manipulated. The general goal of this plugin design is to assist users in creating a variety of realistic sounding impact sounds without the need for a large sample library or synthesis expertise. 
#### Known Issues
- latent controls are not functional
- stutter playback after new file is imported
- importing non-audio files result in program crash

[rave_repo]: https://github.com/acids-ircam/RAVE

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

## Build
1. In the cloned directory
2. `mkdir build; cd build`
3. `cmake -DCMAKE_PREFIX_PATH="your/absolute/path/to/libtorch" ..`
4. `cmake --build . --config Debug` 
7. Copy the DLL files in `simpact_vst\build\AudioPluginExample_artefacts\Debug` to your executable directory (e.g. C:\Program Files\REAPER (x64))

### Feature in progress
- Latent space controls
- Playback randomisation control 
- MIDI note assignment (play the assigned clips on different notes)

### Demo

<a href="https://youtu.be/TYQM8mYBsws"><img src="/images/video_thumbnail.PNG" alt="Demonstration video" width="40%" height="40%" /></a>

#### Contacts
- [Ann Yang][linkedin], annyang.p@gmail.com

Please feel free to reach out to me for any feedback, comments, or install issues! I'll be actively working on this as part of my dissertation project.   

[linkedin]: https://www.linkedin.com/in/annyang-p/
