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
    outputvolume_Slider(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow),
    outputvolume_Attachment(parameterTree, "volume", outputvolume_Slider),
    // latemt control
    latentcontrol1_Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
    latentcontrol1_Attachment(parameterTree, "1-control", latentcontrol1_Slider),
    latentcontrol2_Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
    latentcontrol2_Attachment(parameterTree, "2-control", latentcontrol2_Slider),
    latentcontrol3_Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
    latentcontrol3_Attachment(parameterTree, "3-control", latentcontrol3_Slider),
    latentcontrol4_Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
    latentcontrol4_Attachment(parameterTree, "4-control", latentcontrol4_Slider),
    latentcontrol5_Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
    latentcontrol5_Attachment(parameterTree, "5-control", latentcontrol5_Slider)

{
    // Add the backround to our editor component.
    addAndMakeVisible (background.get());

    // Set up the file chooser button
    fileChooserButton.setButtonText ("File Explorer");
    fileChooserButton.addListener (this);
    fileChooserButton.changeWidthToFitText (50);
    addAndMakeVisible (fileChooserButton);

    addAndMakeVisible(outputvolume_Slider);

    addAndMakeVisible(latentcontrol1_Slider);
    addAndMakeVisible(latentcontrol2_Slider);
    addAndMakeVisible(latentcontrol3_Slider);
    addAndMakeVisible(latentcontrol4_Slider);
    addAndMakeVisible(latentcontrol5_Slider);


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
    // Set the image to take up the entire editor.
    background->setBounds(getLocalBounds());

    // Set the position of the file chooser button in the top left corner.
    fileChooserButton.setBounds(30, 30, 120, 50);

    // Set the position of the volume knob in the far right corner.
    outputvolume_Slider.setBounds(getWidth() - 110, 80, 70, 170);

    // Calculate the horizontal gap between each of the latent control knobs.
    int horizontalGap = (getWidth() - 160) / 4;

    // Set the position of the first latent control knob.
    latentcontrol1_Slider.setBounds(180, 80, 100, 100);

    // Set the position of the second latent control knob.
    latentcontrol2_Slider.setBounds(330, 80, 100, 100);

    // Set the position of the third latent control knob.
    latentcontrol3_Slider.setBounds(120, 220, 100, 100);

    // Set the position of the fourth latent control knob.
    latentcontrol4_Slider.setBounds(270, 220, 100, 100);

    // Set the position of the fifth latent control knob.
    latentcontrol5_Slider.setBounds(420, 220, 100, 100);
}

//==============================================================================

void AudioPluginAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if (button == &fileChooserButton)
    {
        // Handle file chooser button click event here
        juce::FileChooser chooser ("Select a file to open...", {}, "*.wav;*.mp3");
        // TODO: check the selected file is definitely .wav or .mp3
        if (chooser.browseForFileToOpen())
        {
            auto file = chooser.getResult();
            auto newfilePath = file.getFullPathName();
            DBG ("Selected file: " << newfilePath);

            // Check if the file extension is .wav or .mp3
            auto fileExtension = file.getFileExtension();
            if (fileExtension == ".wav" || fileExtension == ".mp3")
            {
                // TODO Pass the file path to the processor
                processorRef.filePath = newfilePath;
                processorRef.newfile.clear();
            }
            else
            {
                // Display an error message if the selected file is not .wav or .mp3
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                         "Error",
                                                         "Please select a .wav or .mp3 file.",
                                                         "OK");
            }
        }
    }
}