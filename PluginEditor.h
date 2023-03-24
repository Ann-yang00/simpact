#pragma once

//#include <JuceHeader.h>
#include "PluginProcessor.h"
//==============================================================================
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Button::Listener
{
public:
    AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&,
                                    juce::AudioProcessorValueTreeState &parameterTree);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    // Our Button::Listener callback. Every time the button is clicked, this function
    // will be called.
    void buttonClicked (juce::Button* button) override;

private:

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    //==============================================================================
    // We will use an image as the background of our editor. Images can be added to
    // the editor using the juce::Drawable class.
    std::unique_ptr <juce::Drawable> background;
    
    // We also want a button to trigger the sample. The juce::TextButton class will
    // give us a button with a textual label on it.
    juce::TextButton fileChooserButton;

    juce::Slider outputvolume_Slider;
    SliderAttachment outputvolume_Attachment; 

    //=============================================================================
    // Declare the latent controls:
    juce::Slider latentcontrol1_Slider;
    SliderAttachment latentcontrol1_Attachment;
    juce::Slider latentcontrol2_Slider;
    SliderAttachment latentcontrol2_Attachment;
    juce::Slider latentcontrol3_Slider;
    SliderAttachment latentcontrol3_Attachment;
    juce::Slider latentcontrol4_Slider;
    SliderAttachment latentcontrol4_Attachment;
    juce::Slider latentcontrol5_Slider;
    SliderAttachment latentcontrol5_Attachment;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
