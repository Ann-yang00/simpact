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
    void setFilePath (const juce::String& newPath);
    // TODO this needs updating
    void AudioPluginAudioProcessor::loadAudioFile();
    std::atomic_flag newfile;


private:

    juce::AudioProcessorValueTreeState parameters;
    std::atomic <float>* output_volume;
    std::atomic_flag changesApplied;
    
    // This function will be called to update the filter coefficients based on the
    // user parameters.
    void updateProcessors();
    void populateParameterValues();

    // file loader
    juce::String filePath;
    
    juce::AudioFormatManager formatManager;

    std::unique_ptr <juce::AudioFormatReader> fileReader1, fileReader2;
    std::unique_ptr <juce::AudioFormatReaderSource> filePlayer1;
    std::unique_ptr <juce::PositionableAudioSource> filePlayer2;
    std::unique_ptr <juce::ResamplingAudioSource> resampler1, resampler2;


    juce::AudioBuffer<float> loadedBuffer;

    // latent space manipulation


    // Encoder
    void AudioPluginAudioProcessor::latentRepresentation();
    std::string rave_model_file = "C:\\Users\\firef\\Documents\\simpact_vst\\rave_impact_model_mono.ts";

    torch::jit::script::Module model;

    const int modelSampleRate = 44100;
    torch::Tensor latent_vectors, decoded_output; //decoded_audio;
    torch::Tensor delta;
    // decoder
    void AudioPluginAudioProcessor::decoder();

    // latent space update
    bool AudioPluginAudioProcessor::mod_latent();
    juce::CriticalSection criticalSection;
    std::vector<std::atomic<float>*> latent_controls;
    std::vector<float> snapshot;
    /*
    std::atomic <float>* latent_control1;
    std::atomic <float>* latent_control2;
    std::atomic <float>* latent_control3;
    std::atomic <float>* latent_control4;
    std::atomic <float>* latent_control5;
    */
    //void fileChooser();

    // Playback
    int bufferReadPosition = 0;
    juce::AudioDeviceManager deviceManager;
    juce::AudioSourcePlayer player;
    float outputGain = 0.0f;

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
