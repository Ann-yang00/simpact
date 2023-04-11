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
    outputvolume_Attachment(parameterTree, "volume", outputvolume_Slider)
{
    // Add the backround to our editor component.
    addAndMakeVisible (background.get());

    // Set up the file chooser button
    fileChooserButton.setButtonText ("File Explorer");
    fileChooserButton.addListener (this);
    fileChooserButton.changeWidthToFitText (50);
    addAndMakeVisible (fileChooserButton);
    addAndMakeVisible(outputvolume_Slider);

    // define the latent controls and attachments in a loop:
    latentcontrol_Sliders.reserve(processorRef.vector_num);
    latentcontrol_Attachments.reserve(processorRef.vector_num);
    latentcontrol_Labels.reserve(processorRef.vector_num);

    for (int i = 0; i < processorRef.vector_num; ++i)
    {
        auto slider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
        auto attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameterTree, std::to_string(i+1) + "-control", *slider);
        auto label = std::make_unique<juce::Label>();
        label->setText("Latent control " + std::to_string(i + 1), juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centred);

        addAndMakeVisible(*slider);
        addAndMakeVisible(*label);

        latentcontrol_Sliders.push_back(std::move(slider));
        latentcontrol_Attachments.push_back(std::move(attachment));
        latentcontrol_Labels.push_back(std::move(label));
    }
    // Not resizable!
    setResizable(false,
        false);

    // To make our editor the same size as the background image we can get the
    // drawable bounds of the image. We then use these to set the editor's size.
    auto bgBounds = background->getDrawableBounds();
    setSize(bgBounds.getWidth(),
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
    fileChooserButton.setBounds(20, 10, 120, 20);

    // Set the position of the volume knob in the far right corner.
    outputvolume_Slider.setBounds(getWidth() - 100, 80, 70, 170);

    int knobWidth = 90;
    int knobHeight = 70;
    int labelHeight = 20;
    int startX = 50;
    int startY = 40;
    int knobsPerRow = 5;
    int columnSpacing = (getWidth() - 2 * startX - knobsPerRow * knobWidth) / (knobsPerRow - 0.3);
    int rowSpacing = (getHeight() - startY - knobHeight) / 10;

    for (int i = 0; i < processorRef.vector_num; ++i)
    {
        int row = i / knobsPerRow;
        int column = i % knobsPerRow;

        int x = startX + column * (knobWidth + columnSpacing);
        int y = startY + row * (knobHeight + rowSpacing);

        latentcontrol_Sliders[i]->setBounds(x, y, knobWidth, knobHeight);
        latentcontrol_Labels[i]->setBounds(x, y - labelHeight, knobWidth, labelHeight);
    }
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