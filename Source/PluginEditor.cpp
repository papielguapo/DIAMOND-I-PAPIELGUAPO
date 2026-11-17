#include "PluginEditor.h"
#include <BinaryData.h>
namespace {
void screw(juce::Graphics& g,float x,float y) {
 g.setColour(juce::Colour(0xffededed)); g.fillEllipse(x-12,y-11,24,24);
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff080a0b),x,y-10,juce::Colour(0xff515657),x,y+10,false));
 g.fillEllipse(x-11,y-11,22,22); g.setColour(juce::Colour(0xff0d1011)); g.fillEllipse(x-7,y-7,14,14);
 g.setColour(juce::Colour(0xff393d3e)); g.drawLine(x-3,y+3,x+3,y-3,2);
}
juce::Image panelImage() {
 juce::Image image(juce::Image::RGB,1252,580,true); juce::Graphics g(image);
 // Adapted from the real Diamond II makePanel(): same brushed metal and rack hardware.
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xffeef0f0),0,0,juce::Colour(0xff8e9294),0,580,false)); g.fillAll();
 for(int y=0;y<580;++y) { g.setColour((y%3==0?juce::Colours::white:juce::Colours::black).withAlpha(y%3==0?.25f:.10f)); g.drawHorizontalLine(y,1,1251); }
 g.setColour(juce::Colours::white.withAlpha(.75f)); g.drawRoundedRectangle(1,1,1250,578,5,2);
 g.setColour(juce::Colour(0xff55595b)); g.drawRoundedRectangle(0,0,1252,580,5,1);
 for(float x:{40.f,1212.f}) {g.setColour(juce::Colours::black.withAlpha(.38f));g.drawVerticalLine((int)x,3,577);g.setColour(juce::Colours::white.withAlpha(.45f));g.drawVerticalLine((int)x+1,3,577);}
 for(float x:{21.f,1231.f}) for(float y:{27.f,553.f}) {g.setColour(juce::Colour(0xff686d6f));g.fillRoundedRectangle(x-14,y-9,28,18,9);g.setColour(juce::Colour(0xff030505));g.fillRoundedRectangle(x-12,y-7,24,14,7);g.setColour(juce::Colours::white.withAlpha(.4f));g.drawLine(x-7,y+8,x+7,y+8,1);}
 for(float x:{62.f,1190.f})for(float y:{75.f,510.f})screw(g,x,y);
 g.setColour(juce::Colour(0xff101316));g.setFont(juce::FontOptions(43.f,juce::Font::bold));g.drawText("DIAMOND I",91,18,400,55,juce::Justification::centredLeft);
 g.setFont(juce::FontOptions(20.f));g.drawText("H A R M O N I C   E M U L A T I O N",500,34,670,30,juce::Justification::centredLeft);
 auto artwork=juce::ImageCache::getFromMemory(BinaryData::rackreference_png,BinaryData::rackreference_pngSize);
 juce::Image signature(juce::Image::ARGB,162,51,true);
 for(int y=0;y<51;++y)for(int x=0;x<162;++x){float alpha=juce::jlimit(0.f,1.f,(.62f-artwork.getPixelAt(1517+x,108+y).getBrightness())*2.5f);signature.setPixelAt(x,y,juce::Colour(0xff050607).withAlpha(alpha));}
 g.drawImage(signature,975,514,180,51,0,0,162,51);
 g.setFont(juce::FontOptions(15.f,juce::Font::bold));g.drawText("PAPIELGUAPO",510,541,230,25,juce::Justification::centred);
 for(int i=0;i<12;++i){int x=91+i*90;g.setFont(juce::FontOptions(21.f,juce::Font::bold));g.drawText("H"+juce::String(i+2),x,94,72,30,juce::Justification::centred);g.setColour(i%2==0?juce::Colour(0xff64168a):juce::Colour(0xff353a40));g.fillRoundedRectangle((float)x+26,128,20,3,1);g.setColour(juce::Colour(0xff101316));
  for(int v=0;v<=100;v+=25){float y=380-v*2.17f;g.drawLine((float)x+19,y,(float)x+25,y,1);g.setFont(juce::FontOptions(10.f));g.drawText(juce::String(v),x-8,(int)y-6,24,12,juce::Justification::centredRight);}
 }
 const char* names[]={"INPUT","OUTPUT","MIX","OVERSAMPLING"};int xs[]={93,323,553,795};
 for(int i=0;i<4;++i){g.setFont(juce::FontOptions(15.f,juce::Font::bold));g.drawText(names[i],xs[i],443,200,24,juce::Justification::centred);}
 return image;
}
}
DiamondLook::DiamondLook() {
 skin=panelImage();
 setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff0d1011));setColour(juce::Slider::textBoxOutlineColourId,juce::Colour(0xff777777));
 setColour(juce::ComboBox::textColourId,juce::Colours::white);setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff202326));setColour(juce::PopupMenu::textColourId,juce::Colours::white);
}
void DiamondLook::drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float,float,juce::Slider::SliderStyle,juce::Slider& s) {
 bool vertical=s.isVertical();float cx=x+w*.5f,cy=y+h*.5f;
 g.setColour(juce::Colours::white.withAlpha(.6f));if(vertical)g.fillRoundedRectangle(cx-5,(float)y,10,(float)h,4);else g.fillRoundedRectangle((float)x,cy-5,(float)w,10,4);
 g.setColour(juce::Colour(0xff080a0b));if(vertical)g.fillRoundedRectangle(cx-3,y+1.f,6,h-2.f,3);else g.fillRoundedRectangle(x+1.f,cy-3,w-2.f,6,3);
 juce::Rectangle<float> thumb=vertical?juce::Rectangle<float>(cx-20,pos-11,40,22):juce::Rectangle<float>(pos-8,cy-12,16,24);
 g.setColour(juce::Colours::black.withAlpha(.3f));g.fillRoundedRectangle(thumb.translated(2,3).expanded(1),3);
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xfffafafa),0,thumb.getY(),juce::Colour(0xff55595b),0,thumb.getBottom(),false));g.fillRoundedRectangle(thumb,3);
 g.setColour(juce::Colour(0xff323538));g.drawRoundedRectangle(thumb,3,1);
 g.setColour(juce::Colour(0xff64168a));if(vertical)g.drawLine(thumb.getX()+3,pos,thumb.getRight()-3,pos,2);else g.drawLine(pos,thumb.getY()+3,pos,thumb.getBottom()-3,2);
}
void DiamondLook::drawComboBox(juce::Graphics& g,int w,int h,bool,int,int,int,int,juce::ComboBox&) {g.setColour(juce::Colour(0xff0d1011));g.fillRoundedRectangle(0,0,(float)w,(float)h,3);g.setColour(juce::Colour(0xff777777));g.drawRoundedRectangle(.5f,.5f,w-1.f,h-1.f,3,1);g.setColour(juce::Colour(0xffa9a1b2));g.drawText("v",w-24,0,20,h,juce::Justification::centred);}
void DiamondLook::drawToggleButton(juce::Graphics& g,juce::ToggleButton& b,bool,bool) {g.setColour(juce::Colour(0xff181b1d));g.fillRoundedRectangle(b.getLocalBounds().toFloat(),4);g.setColour(b.getToggleState()?juce::Colour(0xffb97ad6):juce::Colour(0xff62666a));g.fillEllipse(12,11,10,10);g.setColour(juce::Colours::white);g.setFont(juce::FontOptions(14.f,juce::Font::bold));g.drawText(b.getButtonText(),29,0,b.getWidth()-34,b.getHeight(),juce::Justification::centred);}
DiamondEditor::DiamondEditor(DiamondProcessor& p):AudioProcessorEditor(p),processor(p) {
 setLookAndFeel(&look);addAndMakeVisible(panel);panel.setSize(1252,580);panel.setInterceptsMouseClicks(false,true);
 for(int i=0;i<15;++i){auto& s=sliders[i];panel.addAndMakeVisible(s);s.setSliderStyle(i<12?juce::Slider::LinearVertical:juce::Slider::LinearHorizontal);s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,i<12?66:100,26);s.setTextValueSuffix(i<12||i==14?" %":" dB");s.setDoubleClickReturnValue(true,i==14?100:0);s.setNumDecimalPlacesToDisplay(2);s.setName(i<12?"H"+juce::String(i+2):i==12?"Input":i==13?"Output":"Mix");s.setTooltip(i<12?"H"+juce::String(i+2)+" contribution. Double-click to reset. Calibrated for a 0 dBFS sine.":"Double-click to reset");juce::String id=i<12?"harmonic"+juce::String(i+2):i==12?"inputGain":i==13?"outputGain":"dryWet";attachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,id,s);}
 panel.addAndMakeVisible(oversampling);oversampling.addItemList({"2x","4x","8x"},1);oversampling.setJustificationType(juce::Justification::centred);osAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"oversampling",oversampling);
 panel.addAndMakeVisible(bypass);bypassAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"bypass",bypass);
 setResizable(true,true);setResizeLimits(313,145,2504,1160);getConstrainer()->setFixedAspectRatio(1252./580.);setSize(defaultWidth,defaultHeight);startTimerHz(30);
}
DiamondEditor::~DiamondEditor(){stopTimer();setLookAndFeel(nullptr);}
void DiamondEditor::resized(){panel.setTransform(juce::AffineTransform::scale(getWidth()/1252.f));for(int i=0;i<12;++i)sliders[i].setBounds(91+i*90,150,72,268);for(int i=12;i<15;++i)sliders[i].setBounds(93+(i-12)*230,471,200,61);oversampling.setBounds(822,475,145,32);bypass.setBounds(1030,475,130,32);}
void DiamondEditor::setScalePercent(int percent){const int width=juce::roundToInt(1252*juce::jlimit(25,200,percent)/100.);setSize(width,juce::roundToInt(width*580./1252.));}
void DiamondEditor::mouseDown(const juce::MouseEvent& event){
 if(!event.mods.isPopupMenu())return;
 juce::PopupMenu menu;menu.addSectionHeader("Interface size");
 for(int percent:{25,33,50,60,75,100,125,150,200})menu.addItem(percent,juce::String(percent)+" %",true,getWidth()==juce::roundToInt(1252*percent/100.));
 menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),[safe=juce::Component::SafePointer<DiamondEditor>(this)](int percent){if(safe&&percent>0)safe->setScalePercent(percent);});
}
void DiamondEditor::paint(juce::Graphics& g){g.fillAll(juce::Colours::black);juce::Graphics::ScopedSaveState save(g);g.addTransform(juce::AffineTransform::scale(getWidth()/1252.f));g.drawImageAt(look.skin,0,0);
 for(int meter=0;meter<2;++meter)for(int ch=0;ch<2;++ch){float v=(meter==0?processor.inputPeak[ch]:processor.outputPeak[ch]).load();float db=juce::Decibels::gainToDecibels(v,-60.f),fill=juce::jlimit(0.f,1.f,(db+60)/60);float x=110.f+meter*230,y=535.f+ch*5;g.setColour(juce::Colour(0xff24272a));g.fillRect(x,y,130.f,3.f);g.setColour(db>0?juce::Colour(0xffa42b39):juce::Colour(0xff64168a));g.fillRect(x,y,fill*130,3.f);if(ch==0){g.setColour(juce::Colour(0xff101316));g.setFont(juce::FontOptions(10.f));g.drawText(juce::String(db,1)+" dBFS",(int)x+133,(int)y-5,67,16,juce::Justification::centredLeft);}}
}
