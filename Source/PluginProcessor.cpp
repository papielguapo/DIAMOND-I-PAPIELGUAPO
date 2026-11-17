#include "PluginProcessor.h"
#include "PluginEditor.h"
namespace {
const std::array<juce::String,17> ids={"harmonic2","harmonic3","harmonic4","harmonic5","harmonic6","harmonic7","harmonic8","harmonic9","harmonic10","harmonic11","harmonic12","harmonic13","inputGain","outputGain","dryWet","oversampling","bypass"};
float finite(float x) noexcept { return std::isfinite(x) ? x : 0.f; }
}
juce::AudioProcessorValueTreeState::ParameterLayout DiamondProcessor::layout() {
 juce::AudioProcessorValueTreeState::ParameterLayout l;
 for(int i=0;i<12;++i) l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ids[i],1},"H"+juce::String(i+2),juce::NormalisableRange<float>(0,100,.01f),0.f));
 for(int i=12;i<15;++i) l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ids[i],1},i==12?"Input Gain":i==13?"Output Gain":"Mix",juce::NormalisableRange<float>(i==14?0.f:-24.f,i==14?100.f:24.f,.01f),i==14?100.f:0.f));
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"oversampling",1},"Oversampling",juce::StringArray{"2x","4x","8x"},1));
 l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bypass",1},"Bypass",false));
 return l;
}
DiamondProcessor::DiamondProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"DIAMOND_I",layout()) {
 for(size_t i=0;i<ids.size();++i) parameters[i]=state.getRawParameterValue(ids[i]);
}
bool DiamondProcessor::isBusesLayoutSupported(const BusesLayout& l) const { return l.getMainInputChannelSet()==l.getMainOutputChannelSet() && (l.getMainInputChannelSet()==juce::AudioChannelSet::mono() || l.getMainInputChannelSet()==juce::AudioChannelSet::stereo()); }
void DiamondProcessor::prepareToPlay(double sr,int bs) {
 capacity=juce::jmax(1,bs); controls.setSize(19,capacity); source.setSize(2,capacity); raw.setSize(2,capacity);
 int latency=0;
 for(int k=0;k<3;++k) {
  auto& b=branches[k]; b.factor=1<<(k+1);
  b.os=std::make_unique<juce::dsp::Oversampling<float>>(2);
  b.os->clearOversamplingStages(); b.os->setUsingIntegerLatency(true);
  // High orders magnify passband errors. Do not relax precision at later stages.
  for(int stage=0;stage<=k;++stage) b.os->addOversamplingStage(juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,stage==0?.05f:.10f,-120.f,stage==0?.06f:.12f,-120.f);
  b.os->initProcessing((size_t)capacity); b.scratch.setSize(2,capacity);
  b.latency=juce::roundToInt(b.os->getLatencyInSamples()); latency=juce::jmax(latency,b.latency);
  b.dcPole=std::exp(-2*juce::MathConstants<double>::pi*3/(sr*b.factor));
 }
 for(auto& b:branches) b.pad.prepare(latency-b.latency);
 rawDelay.prepare(latency); gainDelay.prepare(latency); setLatencySamples(latency);
 for(int i=0;i<16;++i) { smooth[i].reset(sr,.025); float v=i<12?parameters[i]->load():i<14?juce::Decibels::decibelsToGain(parameters[i]->load()):i==14?parameters[14]->load()*.01f:1-parameters[16]->load(); smooth[i].setCurrentAndTargetValue(v); }
 const int selected=juce::jlimit(0,2,(int)parameters[15]->load());
 for(int k=0;k<3;++k) { weights[k].reset(sr,.025); weights[k].setCurrentAndTargetValue(k==selected?1.f:0.f); }
 reset();
}
void DiamondProcessor::reset() { rawDelay.clear(); gainDelay.clear(); for(auto& b:branches) { if(b.os) b.os->reset(); b.pad.clear(); b.dc.fill(0); } for(auto& p:inputPeak)p=0;for(auto& p:outputPeak)p=0; }
void DiamondProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&) { process(b,false); }
void DiamondProcessor::processBlockBypassed(juce::AudioBuffer<float>& b,juce::MidiBuffer&) { process(b,true); }
void DiamondProcessor::process(juce::AudioBuffer<float>& buffer,bool hostBypass) {
 juce::ScopedNoDenormals nd;
 if(capacity==0) { buffer.clear(); return; }
 const int channels=juce::jmin(2,buffer.getNumChannels());
 for(int i=0;i<12;++i) smooth[i].setTargetValue(parameters[i]->load());
 for(int i=12;i<14;++i) smooth[i].setTargetValue(juce::Decibels::decibelsToGain(parameters[i]->load()));
 smooth[14].setTargetValue(parameters[14]->load()*.01f); smooth[15].setTargetValue(hostBypass?0.f:1-parameters[16]->load());
 const int selected=juce::jlimit(0,2,(int)parameters[15]->load());
 for(int k=0;k<3;++k) weights[k].setTargetValue(k==selected?1.f:0.f);
 float ip[2]{},op[2]{};
 for(int offset=0;offset<buffer.getNumSamples();offset+=capacity) {
  const int count=juce::jmin(capacity,buffer.getNumSamples()-offset);
  for(int j=0;j<count;++j) {
   for(int i=0;i<16;++i) controls.setSample(i,j,smooth[i].getNextValue());
   for(int k=0;k<3;++k) controls.setSample(16+k,j,weights[k].getNextValue());
   for(int ch=0;ch<2;++ch) {
    float x=ch<channels?finite(buffer.getSample(ch,offset+j)):0.f; ip[ch]=juce::jmax(ip[ch],std::abs(x));
    raw.setSample(ch,j,rawDelay.tick(ch,x));
    source.setSample(ch,j,finite(x*controls.getSample(12,j)));
   }
   rawDelay.advance();
  }
  for(auto& b:branches) {
   for(int ch=0;ch<2;++ch) b.scratch.copyFrom(ch,0,source,ch,0,count);
   auto block=juce::dsp::AudioBlock<float>(b.scratch).getSubBlock(0,(size_t)count);
   auto up=b.os->processSamplesUp(block);
   for(size_t j=0;j<up.getNumSamples();++j) {
    std::array<float,12> amount{}; for(int i=0;i<12;++i) amount[i]=controls.getSample(i,(int)j/b.factor);
    for(int ch=0;ch<2;++ch) {
     double r=diamond::residual(up.getChannelPointer(ch)[j],amount);
     b.dc[ch]=(1-b.dcPole)*r+b.dcPole*b.dc[ch];
     up.getChannelPointer(ch)[j]=finite((float)(r-b.dc[ch]));
    }
   }
   b.os->processSamplesDown(block);
   for(int j=0;j<count;++j) { for(int ch=0;ch<2;++ch) b.scratch.setSample(ch,j,b.pad.tick(ch,b.scratch.getSample(ch,j))); b.pad.advance(); }
  }
  for(int j=0;j<count;++j) {
   for(int ch=0;ch<2;++ch) {
    float clean=gainDelay.tick(ch,source.getSample(ch,j)),delta=0;
    for(int k=0;k<3;++k) delta+=branches[k].scratch.getSample(ch,j)*controls.getSample(16+k,j);
    float dry=raw.getSample(ch,j),active=(clean+delta*controls.getSample(14,j))*controls.getSample(13,j);
    float y=finite(dry+controls.getSample(15,j)*(active-dry));
    if(ch<channels) buffer.setSample(ch,offset+j,y);
    op[ch]=juce::jmax(op[ch],std::abs(y));
   }
   gainDelay.advance();
  }
 }
 for(int ch=0;ch<2;++ch) { inputPeak[ch].store(ip[ch]); outputPeak[ch].store(op[ch]); }
}
juce::AudioProcessorEditor* DiamondProcessor::createEditor() { return new DiamondEditor(*this); }
void DiamondProcessor::getStateInformation(juce::MemoryBlock& b) { auto tree=state.copyState(); auto xml=tree.createXml(); copyXmlToBinary(*xml,b); }
void DiamondProcessor::setStateInformation(const void* data,int size) {
 if(auto xml=getXmlFromBinary(data,size)) if(xml->hasTagName("DIAMOND_I")) {
  auto tree=juce::ValueTree::fromXml(*xml);
  for(auto child:tree) { auto id=child.getProperty("id").toString(); if(auto* p=state.getParameter(id)) { float v=(float)child.getProperty("value"); if(std::isfinite(v)) p->setValueNotifyingHost(p->convertTo0to1(juce::jlimit(p->getNormalisableRange().start,p->getNormalisableRange().end,v))); } }
 }
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DiamondProcessor; }
