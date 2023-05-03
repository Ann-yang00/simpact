#pragma once

//#include <JuceHeader.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <torch/script.h>
#include <torch/torch.h>
#include <juce_core/juce_core.h>

//==============================================================================
class AudioPluginAudioProcessor  : public juce::AudioProcessor,
                                   public juce::ValueTree::Listener
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void valueTreePropertyChanged (juce::ValueTree &treeWhosePropertyHasChanged,
                                   const juce::Identifier &property) override;

    //==============================================================================
    // Editor file selction
    std::atomic_flag newfile;
    juce::String filePath;

private:
    // Load resources
    torch::jit::script::Module model;
    std::string rave_model_file = juce::File(juce::String(__FILE__)).getParentDirectory().getFullPathName().toStdString() + "/rave_impact_model_mono.ts";
    std::string default_audio_file = juce::File(juce::String(__FILE__)).getParentDirectory().getFullPathName().toStdString() + "/foley_footstep_single_metal_ramp.wav";
    const int modelSampleRate = 44100;

    // Parameters
    juce::AudioProcessorValueTreeState parameters;
    std::atomic <float>* output_volume;
    std::atomic <float>* rand_control;
    std::atomic_flag changesApplied;
    void populateParameterValues();
   
    // Re-sampling and playback
    void loadAudioFile();
    int bufferReadPosition = 0;
    float outputGain = 0.0f;

    juce::AudioFormatManager formatManager;
    std::unique_ptr <juce::AudioFormatReader> fileReader1;
    std::unique_ptr <juce::AudioFormatReaderSource> filePlayer1;
    std::unique_ptr <juce::PositionableAudioSource> filePlayer2;
    std::unique_ptr <juce::ResamplingAudioSource> resampler1, resampler2;
    juce::AudioBuffer<float> loadedBuffer;

    // Latent control & model functions
    void updateProcessors();
    void mod_latent();
    std::vector<std::atomic<float>*> latent_controls;

    void encoder();
    void decoder();
    torch::Tensor latent_vectors, decoded_output, encoded_input; 
    torch::Tensor delta, index;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor);
};

class BufferAudioSource : public juce::PositionableAudioSource
{
public:
    BufferAudioSource(juce::AudioBuffer<float>& buffer)
        : buffer(buffer), readPosition(0) {}

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
    }

    void releaseResources() override
    {
    }
    
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        int numSamples = bufferToFill.numSamples;
        int position = bufferToFill.startSample;

        int samplesAvailable = buffer.getNumSamples() - readPosition;

        // Check if the requested number of samples is available
        if (samplesAvailable < numSamples)
        {
            numSamples = samplesAvailable;
            bufferToFill.clearActiveBufferRegion();
        }

        // If there are no more samples, return early
        if (numSamples <= 0)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            bufferToFill.buffer->copyFrom(channel, position, buffer, channel, readPosition, numSamples);
        }

        readPosition += numSamples;
    }

    void setNextReadPosition(juce::int64 newPosition) override
    {
        readPosition = static_cast<int>(newPosition);
    }

    juce::int64 getNextReadPosition() const override
    {
        return readPosition;
    }

    juce::int64 getTotalLength() const override
    {
        return buffer.getNumSamples();
    }

    bool isLooping() const override
    {
        return false;
    }

    void setLooping(bool shouldLoop) override
    {
    }

private:
    juce::AudioBuffer<float>& buffer;
    int readPosition;
};
