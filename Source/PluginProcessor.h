#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "Harmonics.h"
class DiamondProcessor final : public juce::AudioProcessor {
public:
 DiamondProcessor();
 const juce::String getName() const override { return "DIAMOND I - HARMONIC EMULATION"; }
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 void reset() override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 void processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override { return true; }
 bool acceptsMidi() const override { return false; }
 bool producesMidi() const override { return false; }
 double getTailLengthSeconds() const override { return 1.0; }
 int getNumPrograms() override { return 1; }
 int getCurrentProgram() override { return 0; }
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override { return "Clean"; }
 void changeProgramName(int,const juce::String&) override {}
 juce::AudioProcessorParameter* getBypassParameter() const override { return state.getParameter("bypass"); }
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState state;
 std::array<std::atomic<float>,2> inputPeak{},outputPeak{};
private:
 struct Delay {
  juce::AudioBuffer<float> ring; int pos=0,length=0;
  void prepare(int n) { length=n; pos=0; ring.setSize(2,juce::jmax(1,n)); ring.clear(); }
  float tick(int ch,float x) noexcept { if(length==0) return x; float y=ring.getSample(ch,pos); ring.setSample(ch,pos,x); return y; }
  void advance() noexcept { if(length>0 && ++pos==length) pos=0; }
  void clear() { ring.clear(); pos=0; }
 } rawDelay,gainDelay;
 struct Branch {
  std::unique_ptr<juce::dsp::Oversampling<float>> os;
  juce::AudioBuffer<float> scratch;
  Delay pad;
  std::array<double,2> dc{};
  double dcPole=0; int factor=2,latency=0;
 };
 std::array<Branch,3> branches;
 std::array<std::atomic<float>*,17> parameters{};
 std::array<juce::SmoothedValue<float>,16> smooth;
 std::array<juce::SmoothedValue<float>,3> weights;
 juce::AudioBuffer<float> controls,source,raw;
 int capacity=0;
 void process(juce::AudioBuffer<float>&,bool);
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
};
