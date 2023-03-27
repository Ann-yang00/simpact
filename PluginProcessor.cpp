#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <sstream>
#include <c10/core/InferenceMode.h>

static const int vector_num = 5;
static const juce::String controlIdSuffix = "-control";
static const juce::String controlNameSuffix = " Control";

static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout parameters;
    juce::NormalisableRange <float> freqRange(20.0f, 20000.0f);
    // adding parameters to relevant groups
    auto playback_group = std::make_unique <juce::AudioProcessorParameterGroup> ("playbackcontrol",
                                                                            "Playback Control",
                                                                            "|");
    playback_group->addChild (std::make_unique <juce::AudioParameterFloat> ("volume",
                                                                                "Volume",
                                                                                -10.0f,
                                                                                30.0f,
                                                                                0.0f));
    parameters.add(std::move(playback_group));                                                        

    auto latent_group = std::make_unique <juce::AudioProcessorParameterGroup>("latentcontrol",
                                                                                "Latent Space Control",
                                                                                "|");
    for (int i = 0; i < vector_num; ++i)
    {
        int vectorNumber = i + 1;
        auto controlID = juce::String(vectorNumber);
        latent_group->addChild(std::make_unique <juce::AudioParameterFloat>(controlID + controlIdSuffix,
                                                                            controlID + controlNameSuffix,
                                                                            -10.0f,
                                                                            10.0f,
                                                                            0.0f));
    }
    parameters.add(std::move(latent_group));

    return parameters;
}

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : parameters (*this, nullptr, "parameters", createParameterLayout()),
      AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::mono(), true))
{
    juce::ScopedLock lock(criticalSection);
    parameters.state.addListener (this); // monitor parameter change
    populateParameterValues();
    changesApplied.clear ();

    formatManager.registerBasicFormats();
    // TODO change this to JUCEfileChooser in the future
    fileReader1.reset (formatManager.createReaderFor (std::make_unique <juce::MemoryInputStream> (BinaryData::foley_footstep_single_metal_ramp_wav,
                                                                                                 BinaryData::foley_footstep_single_metal_ramp_wavSize,
                                                                                                 false)));
    // load model
    c10::InferenceMode guard;
    torch::jit::getProfilingMode() = false;
    torch::jit::setGraphExecutorOptimize(true);

    try {
        // Load the model
        model = torch::jit::load(rave_model_file);
    }
    catch (const c10::Error& e) {
        std::cout << "Error loading the model: " << e.what() << std::endl;
    }
    //auto named_buffers = model.named_buffers();

    loadAudioFile();

}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
void AudioPluginAudioProcessor::latentRepresentation()
{
    juce::ScopedLock lock(criticalSection);
    // Get a pointer to the raw audio data
    const float* audioData = loadedBuffer.getReadPointer(0);

    // Get the number of samples in the buffer
    int numSamples = loadedBuffer.getNumSamples();

    // Create a std::vector to store the audio data
    std::vector<float> audioVector(audioData, audioData + numSamples);
    
    // Create a torch::Tensor from the std::vector
    torch::Tensor audioTensor = torch::from_blob(audioVector.data(), {1, 1, numSamples}, torch::kFloat32);

    // Wrap the torch::Tensor in a c10::IValue
    torch::jit::IValue audioIValue = audioTensor;

    // Add the c10::IValue to a std::vector<c10::IValue>
    std::vector<torch::jit::IValue> model_inputs;
    model_inputs.push_back(audioIValue);

    c10::InferenceMode guard;
    latent_vectors = model.get_method("encode")(model_inputs).toTensor();
    //auto encodeFunc = model.get_method("forward");
    //auto something = encodeFunc(model_inputs);
    //auto decoded_audio = something.toTensor();

    //decoded_audio = model.forward({ latent_vectors }).toTensor();
    decoder();
    
    auto output_shape = decoded_output.sizes();
    int output_num_samples = output_shape[2]; // Assuming the shape is {1, numChannels, numSamples}
    loadedBuffer.setSize(1, output_num_samples, false, true, true);
    auto output_data = decoded_output.view({ 1, output_num_samples }).data_ptr<float>();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        loadedBuffer.setSample(0, sample, output_data[sample]);
    }
}
void AudioPluginAudioProcessor::decoder() {
    juce::ScopedLock lock(criticalSection);

    // Wrap the torch::Tensor in a c10::IValue
    //if (delta.numel() != 0) {
    //    latent_vectors += delta;
    //    delta.zero_();
    //}
    torch::jit::IValue latentIValue = latent_vectors;
    // Add the c10::IValue to a std::vector<c10::IValue>
    std::vector<torch::jit::IValue> decoder_inputs;
    decoder_inputs.push_back(latentIValue);
    decoded_output = model.get_method("decode")(decoder_inputs).toTensor();

}

void AudioPluginAudioProcessor::loadAudioFile()
{      
    juce::ScopedLock lock(criticalSection);
    // Create an AudioFormatReaderSource for the fileReader1
    filePlayer1 = std::make_unique <juce::AudioFormatReaderSource> (fileReader1.get(), false);
    
    // resample uploaded audio 
    double resamplingRatio1 = modelSampleRate / fileReader1->sampleRate;
    resampler1 = std::make_unique<juce::ResamplingAudioSource>(filePlayer1.get(), false, 1);
    resampler1->setResamplingRatio(resamplingRatio1);
    
    // Resize the loadedBuffer to store the resampled audio data
    int newNumSamples = static_cast<int>(fileReader1->lengthInSamples * resamplingRatio1);
    loadedBuffer.setSize(1, newNumSamples, false, true, true);

    // Read the resampled audio data into the loadedBuffer
    resampler1->prepareToPlay(512, fileReader1->sampleRate);
    int samplesToRead = newNumSamples;
    int startSample = 0;
    while (samplesToRead > 0)
    {
        juce::AudioSourceChannelInfo info(&loadedBuffer, startSample, samplesToRead);
        resampler1->getNextAudioBlock(info);
        samplesToRead -= info.numSamples;
        startSample += info.numSamples;
    }

    latentRepresentation();
}

torch::Tensor AudioPluginAudioProcessor::mod_latent() {
    juce::ScopedLock lock(criticalSection);
    bool change = false;
    for (int i = 0; i < vector_num; ++i) {
        auto current_val = latent_controls[i]->load();
        if (current_val != snapshot[i]) {
            change = true;
            delta = torch::zeros_like(latent_vectors);
            torch::Tensor addition = torch::tensor(current_val);
            //auto delta = torch::linspace(current_val, current_val, latent_vectors.size(-1), torch::kFloat32);
            for (int n = 0; n < delta.size(-1); ++n) {
                delta[0][i][n] += addition;
            }
            //latent_vectors.narrow(1, 0, latent_vectors.size(-1)).add_(delta);

            //latent_vectors.slice(1, i, i + 1).squeeze(1) += delta;
            snapshot[i] = current_val;
        }
    }
    if (change == true) {
        updateApplied.clear();
    }
    return delta;
}
//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{   
    //juce::ScopedLock lock(criticalSection);
    //if (!updateApplied.test_and_set()) {
    //    latent_vectors += delta;
    //    decoder();
    //}
    // for playback
    filePlayer2 = std::make_unique<BufferAudioSource>(loadedBuffer);
    resampler2 = std::make_unique<juce::ResamplingAudioSource>(filePlayer2.get(), false, 1);

    // Set the resampler2's output sample rate
    filePlayer2->prepareToPlay(samplesPerBlock, sampleRate);
    double resamplingRatio2 = modelSampleRate / sampleRate;
    resampler2->setResamplingRatio(resamplingRatio2);
    resampler2->prepareToPlay(samplesPerBlock, sampleRate);

    // Initialise the trigger and gain so that the sample won't be played immediately.
    outputGain = 0.0f;
    changesApplied.clear(); // we will reset everything before playback everytime
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    updateProcessors();

    juce::ScopedNoDenormals noDenormals;

    // Process MIDI messages
    juce::MidiMessage midiMessage;
    int sampleNumber;
    for (juce::MidiBuffer::Iterator i(midiMessages); i.getNextEvent(midiMessage, sampleNumber);)
    {
        if (midiMessage.isNoteOn())
        {
            // Start playing the audio clip
            filePlayer2->setNextReadPosition (0);
            outputGain = 1.0f;
        }
    }
    
    // Create an AudioSourceChannelInfo object for resampler2
    resampler2->getNextAudioBlock (juce::AudioSourceChannelInfo (buffer));

    // Apply the gain to silence the initial playback.
    buffer.applyGain(juce::Decibels::decibelsToGain <float>(*output_volume));
    buffer.applyGain (outputGain);
}

void AudioPluginAudioProcessor::releaseResources()
{
    resampler1->releaseResources();
    filePlayer1->releaseResources();
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // only outputs mono
    //return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono(); 
    return true;
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor(*this, parameters);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
 
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
        }
    }
    
    // Clear the changesApplied flag so we don't miss the newly set state information
    changesApplied.clear();
}
//==================================================================================
// See the updateProcessors() function to see how this is picked up and
// processed by the audio thread.
void AudioPluginAudioProcessor::valueTreePropertyChanged (juce::ValueTree &treeWhosePropertyHasChanged,
                                                          const juce::Identifier &property)
{
    juce::ScopedLock lock(criticalSection);
    // Clear the changesApplied flag so that coefficients will be recalcualted on the next
    // call to processBlock(). THe clear() function of std::actomic_flag puts the flag
    // into the clear (false) state.
    changesApplied.clear();
}

void AudioPluginAudioProcessor::updateProcessors()
{
    juce::ScopedLock lock(criticalSection);
    // return if no parameter has been changed
    if (changesApplied.test_and_set()){
        return;
    }
    output_volume = parameters.getRawParameterValue("volume");
    //populateParameterValues();
    delta = mod_latent();
    //if (update == true) {
        //latent_vectors += delta;
        //decoder();
    //}
}

void AudioPluginAudioProcessor::populateParameterValues()
{
    juce::ScopedLock lock(criticalSection);
    latent_controls.reserve(vector_num);
    snapshot.reserve(vector_num);
    // providing raw parameter to the pointers declared 
    output_volume = parameters.getRawParameterValue("volume");

    for (int i = 0; i < vector_num; ++i)
    {
        int vectorNumber = i + 1;
        auto controlID = juce::String(vectorNumber);
        auto rawParameterValue = parameters.getRawParameterValue(controlID + controlIdSuffix);
        latent_controls.push_back(rawParameterValue);
        snapshot.push_back(latent_controls[i]->load());
    }

}
//==============================================================================
// pass new file path
void AudioPluginAudioProcessor::setFilePath (const juce::String& newPath)
    {
        filePath = newPath;
        fileReader1.reset(formatManager.createReaderFor(juce::File(filePath)));
        loadAudioFile();
    }

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const // changed this to take MIDI on 
{
    return true;   
}

bool AudioPluginAudioProcessor::producesMidi() const
{
    return false;
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
    return false;
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
