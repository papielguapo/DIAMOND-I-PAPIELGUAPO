#pragma once
#include "PluginProcessor.h"
class DiamondLook final : public juce::LookAndFeel_V4 {
public:
 DiamondLook();
 void drawLinearSlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider::SliderStyle,juce::Slider&) override;
 void drawComboBox(juce::Graphics&,int,int,bool,int,int,int,int,juce::ComboBox&) override;
 void drawToggleButton(juce::Graphics&,juce::ToggleButton&,bool,bool) override;
 juce::Image skin;
};
class DiamondEditor final : public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit DiamondEditor(DiamondProcessor&);
 ~DiamondEditor() override;
 void paint(juce::Graphics&) override;
 void resized() override;
 void mouseDown(const juce::MouseEvent&) override;
 void setScalePercent(int percent);
 static constexpr int defaultWidth=626,defaultHeight=290;
private:
 DiamondProcessor& processor;
 DiamondLook look;
 juce::Component panel;
 std::array<juce::Slider,15> sliders;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,15> attachments;
 juce::ComboBox oversampling;
 juce::ToggleButton bypass{"BYPASS"};
 std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osAttachment;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
 void timerCallback() override { repaint(); }
};
