//#ifndef JUCE_MODAL_LOOPS_PERMITTED
//#define JUCE_MODAL_LOOPS_PERMITTED 1
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"
//#endif

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p,
    juce::AudioProcessorValueTreeState& parameterTree)
    : 
    AudioProcessorEditor (&p), 
    processorRef (p),
    // Initialise our background Drawable using the image data from BinaryData. 
    background (juce::Drawable::createFromImageData (BinaryData::background_png,
                                                     BinaryData::background_pngSize)),
    output_volumeSlider (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
    output_volumeAttachment (parameterTree, "volume", output_volumeSlider)
{
    // Add the backround to our editor component.
    addAndMakeVisible (background.get());

    // Set up the file chooser button
    fileChooserButton.setButtonText ("File Explorer");
    fileChooserButton.addListener (this);
    fileChooserButton.changeWidthToFitText (50);
    addAndMakeVisible (fileChooserButton);

    addAndMakeVisible(output_volumeSlider);

    // Not resizable!
    setResizable (false, 
                  false); 

    // To make our editor the same size as the background image we can get the
    // drawable bounds of the image. We then use these to set the editor's size.
    auto bgBounds = background->getDrawableBounds();
    setSize (bgBounds.getWidth(),
            bgBounds.getHeight());
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Nothing to do here as our image Drawable will serve as the editor's background.
}

void AudioPluginAudioProcessorEditor::resized()
{
    // Set the image to take up the entirty of the editor.
    background->setBounds (getLocalBounds());
    
    // Set the position of the button. I chose these values by opening the background
    // image in an image editor and finding the pixel positions I wanted the top
    // left of the button to be at.
    fileChooserButton.setTopLeftPosition (270, 180);
    output_volumeSlider.setBounds(100, 180, 100, 100);
}

//==============================================================================

void AudioPluginAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if (button == &fileChooserButton)
    {
        // Handle file chooser button click event here
        juce::FileChooser chooser ("Select a file to open...", {}, "*.wav;*.mp3");
        if (chooser.browseForFileToOpen())
        {
            auto file = chooser.getResult();
            auto filePath = file.getFullPathName();
            DBG ("Selected file: " << filePath);
            // Pass the file path to the processor
            processorRef.setFilePath (filePath);
            //processorRef.newfile.clear();
        }
    }
}
