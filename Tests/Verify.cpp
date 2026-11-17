#include "../Source/PluginEditor.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <new>
#include <chrono>
namespace { thread_local bool track=false; thread_local size_t allocations=0; }
void* operator new(std::size_t n){if(track)++allocations;if(auto p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p) noexcept {std::free(p);}
void operator delete[](void* p) noexcept {std::free(p);}
void operator delete(void* p,std::size_t) noexcept {std::free(p);}
void operator delete[](void* p,std::size_t) noexcept {std::free(p);}
namespace {
int failures=0;
void check(bool ok,const char* text){std::cout<<(ok?"PASS ":"FAIL ")<<text<<"\n";if(!ok)++failures;}
void set(DiamondProcessor& p,const juce::String& id,float v){auto* a=p.state.getParameter(id);a->setValueNotifyingHost(a->convertTo0to1(v));}
std::vector<float> render(DiamondProcessor& p,double sr,double freq,int n,double amplitude=1,bool right=true){
 juce::AudioBuffer<float>b(2,257);juce::MidiBuffer midi;std::vector<float> out((size_t)n);
 for(int offset=0;offset<n;offset+=257){int count=juce::jmin(257,n-offset);b.setSize(2,count,false,false,true);for(int j=0;j<count;++j){float x=(float)(amplitude*std::cos(juce::MathConstants<double>::twoPi*freq*(offset+j)/sr));b.setSample(0,j,x);b.setSample(1,j,right?x:0);}
  p.processBlock(b,midi);for(int j=0;j<count;++j)out[(size_t)offset+j]=b.getSample(0,j);
 }
 return out;
}
double magnitude(const std::vector<float>& x,int start,int n,double f,double sr){double re=0,im=0;for(int j=0;j<n;++j){double a=juce::MathConstants<double>::twoPi*f*j/sr;re+=x[(size_t)start+j]*std::cos(a);im-=x[(size_t)start+j]*std::sin(a);}return 2*std::hypot(re,im)/n;}
double db(double x){return 20*std::log10(std::max(1e-15,x));}
double fftPeak(const std::vector<float>& x,int start,int order,double sr,double target,double* peakFrequency=nullptr){int n=1<<order;std::vector<float> data((size_t)n*2,0);double sum=0;for(int j=0;j<n;++j){double w=.5-.5*std::cos(juce::MathConstants<double>::twoPi*j/(n-1));data[j]=(float)(x[(size_t)start+j]*w);sum+=w;}juce::dsp::FFT fft(order);fft.performFrequencyOnlyForwardTransform(data.data());int center=juce::roundToInt(target*n/sr),best=juce::jlimit(1,n/2-1,center);for(int k=std::max(1,center-2);k<=std::min(n/2-1,center+2);++k)if(data[k]>data[best])best=k;if(peakFrequency)*peakFrequency=best*sr/n;return 2*data[best]/sum;}
}
void snapshots(DiamondProcessor& p,const juce::File& folder){
 DiamondEditor editor(p);for(int width:{313,626,751,1252,1878,2504}){editor.setSize(width,juce::roundToInt(width*580./1252));auto shot=editor.createComponentSnapshot(editor.getLocalBounds());juce::FileOutputStream out(folder.getChildFile("interface-"+juce::String(width)+".png"));if(out.openedOk()){out.setPosition(0);out.truncate();}check(out.openedOk()&&juce::PNGImageFormat{}.writeImageToStream(shot,out),"Interface snapshot exported");}
}
void verifyEditorSizing(){
 DiamondProcessor p;
 {DiamondEditor editor(p);check(editor.getWidth()==626&&editor.getHeight()==290,"Editor opens at 50 percent (626x290)");
  for(int percent:{25,33,50,60,75,100,125,150,200}){editor.setScalePercent(percent);check(editor.getWidth()==juce::roundToInt(1252*percent/100.)&&editor.getHeight()==juce::roundToInt(editor.getWidth()*580./1252.),"Size menu preserves aspect ratio");}
  editor.setScalePercent(1);check(editor.getWidth()==313,"Size lower bound 25 percent");editor.setScalePercent(999);check(editor.getWidth()==2504,"Size upper bound 200 percent");
 }
 {DiamondEditor reopened(p);check(reopened.getWidth()==626&&reopened.getHeight()==290,"Closing an enlarged editor reopens small");}
 juce::XmlElement legacy("DIAMOND_I");legacy.setAttribute("editorWidth",2504);juce::MemoryBlock data;juce::AudioProcessor::copyXmlToBinary(legacy,data);p.setStateInformation(data.getData(),(int)data.getSize());
 {DiamondEditor restored(p);check(restored.getWidth()==626&&restored.getHeight()==290,"Legacy state cannot force a large opening size");}
}
int main(int argc,char** argv){
 std::cout<<std::unitbuf;
 juce::ScopedJuceInitialiser_GUI gui;juce::File folder(argc>1?juce::String(argv[1]):juce::String("docs"));folder.createDirectory();
 verifyEditorSizing();
 if(argc>2&&juce::String(argv[2])=="--screenshots-only"){DiamondProcessor preview;set(preview,"harmonic2",37);set(preview,"harmonic13",63);set(preview,"inputGain",-3);set(preview,"oversampling",2);snapshots(preview,folder);return failures==0?0:1;}
 std::ofstream csv(folder.getChildFile("spectral.csv").getFullPathName().toStdString());csv<<"sample_rate,oversampling,harmonic,expected_hz,fft_peak_hz,target_dBFS,max_other_harmonic_dBc\n";
 for(double sr:{44100.,48000.,96000.})for(int os=0;os<3;++os){
  double worst=-300;bool all=true;
  for(int h=2;h<=13;++h){DiamondProcessor p;set(p,"oversampling",(float)os);set(p,"harmonic"+juce::String(h),100);p.prepareToPlay(sr,256);const int start=(int)sr,n=(int)sr;auto x=render(p,sr,100,start+std::max(n,32768));double a=magnitude(x,start,n,h*100.,sr),other=0;for(int k=2;k<=20;++k)if(k!=h)other=std::max(other,magnitude(x,start,n,k*100.,sr));double f=0;double peak=fftPeak(x,start,15,sr,h*100.,&f);double residual=db(other/a);worst=std::max(worst,residual);all &= std::abs(db(a*h))<.5 && peak>a*.8 && std::abs(f-h*100)<sr/32768. && residual<-65;
   csv<<sr<<","<<(1<<(os+1))<<","<<h<<","<<h*100<<","<<f<<","<<db(a)<<","<<residual<<"\n";
  }
  std::cout<<"Spectral "<<sr<<" Hz "<<(1<<(os+1))<<"x worst residual "<<worst<<" dBc\n";check(all,"H2-H13 100 Hz FFT frequency, calibrated gain and residual harmonics");
  DiamondProcessor clean;set(clean,"oversampling",(float)os);clean.prepareToPlay(sr,256);auto x=render(clean,sr,100,(int)sr);double error=0;for(int j=clean.getLatencySamples();j<(int)x.size();++j){float expected=(float)std::cos(juce::MathConstants<double>::twoPi*100*(j-clean.getLatencySamples())/sr);error=std::max(error,std::abs((double)x[j]-expected));}check(error<1e-7,"All harmonics zero: exact delayed clean sine");
 }
 // High-tone folded 13th: 13*7000 folds to 5000 Hz at 48 kHz.
 std::ofstream alias(folder.getChildFile("aliasing.csv").getFullPathName().toStdString());alias<<"sample_rate,oversampling,input_hz,folded_H13_hz,folded_dBFS\n";
 for(double sr:{44100.,48000.,96000.})for(int os=0;os<3;++os){DiamondProcessor p;set(p,"harmonic13",100);set(p,"oversampling",(float)os);p.prepareToPlay(sr,256);const double f=sr==96000?11000:7000;auto x=render(p,sr,f,(int)sr*2);double folded=std::abs(std::remainder(13*f,sr));double a=magnitude(x,(int)sr,(int)sr,folded,sr);alias<<sr<<","<<(1<<(os+1))<<","<<f<<","<<folded<<","<<db(a)<<"\n";check(std::isfinite(a)&&(os==0||db(a)<-75),"High-tone alias measurement: 4x/8x folded component below -75 dBFS");}
 DiamondProcessor p;set(p,"harmonic2",37);set(p,"harmonic13",63);set(p,"inputGain",-3);set(p,"oversampling",2);juce::MemoryBlock state;p.getStateInformation(state);DiamondProcessor restored;restored.setStateInformation(state.getData(),(int)state.getSize());bool same=true;for(int i=0;i<p.getParameters().size();++i)same &= p.getParameters()[i]->getValue()==restored.getParameters()[i]->getValue();check(same,"All 17 audio parameters persist");
 char bad[32]{};restored.setStateInformation(bad,32);check(restored.state.getRawParameterValue("harmonic13")->load()==63,"Invalid state rejected");
 p.prepareToPlay(48000,128);juce::AudioBuffer<float>b(2,1025);juce::MidiBuffer midi;b.clear();p.processBlock(b,midi);allocations=0;track=true;
 for(int k=0;k<20;++k){b.clear();b.setSample(0,0,.3f);p.processBlock(b,midi);}track=false;check(allocations==0,"No C++ heap allocations in oversized processBlock");check(b.getMagnitude(1,0,b.getNumSamples())==0,"Silent right channel remains silent");
 b.clear();b.setSample(0,0,std::numeric_limits<float>::infinity());b.setSample(1,0,std::numeric_limits<float>::quiet_NaN());p.processBlock(b,midi);bool finite=true;for(int c=0;c<2;++c)for(int j=0;j<b.getNumSamples();++j)finite &= std::isfinite(b.getSample(c,j));check(finite,"NaN and infinity inputs contained");
 const int fixedLatency=p.getLatencySamples();
 for(int k=0;k<3;++k){set(p,"oversampling",(float)k);b.clear();p.processBlock(b,midi);}check(p.getLatencySamples()==fixedLatency&&fixedLatency>0,"Oversampling switches retain fixed latency");
 for(double sr:{44100.,48000.,96000.}){p.prepareToPlay(sr,128);b.clear();p.processBlock(b,midi);check(b.getMagnitude(0,0,b.getNumSamples())==0,"Sample rate re-prepare clears previous DSP history");}
 {DiamondProcessor a,c,combined,clean;for(auto* q:{&a,&c,&combined,&clean})q->prepareToPlay(48000,256);set(a,"harmonic2",75);set(c,"harmonic13",60);set(combined,"harmonic2",75);set(combined,"harmonic13",60);auto x=render(a,48000,100,96000),y=render(c,48000,100,96000),z=render(combined,48000,100,96000),d=render(clean,48000,100,96000);double maxError=0;for(size_t i=48000;i<z.size();++i)maxError=std::max(maxError,std::abs((double)z[i]-(x[i]+y[i]-d[i])));check(maxError<1e-5,"Combined generators equal independent contributions without shared gain changes");}
 {DiamondProcessor perf;for(int h=2;h<=13;++h)set(perf,"harmonic"+juce::String(h),50);perf.prepareToPlay(96000,256);juce::AudioBuffer<float> block(2,256);block.clear();auto begin=std::chrono::steady_clock::now();for(int k=0;k<375;++k){for(int j=0;j<256;++j){float x=.5f*std::sin((float)(j+k*256)*.071f);block.setSample(0,j,x);block.setSample(1,j,x);}perf.processBlock(block,midi);}double elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count();std::cout<<"BENCHMARK 96 kHz stereo, 12 generators, 1 audio second: "<<elapsed<<" seconds CPU wall time; latency "<<perf.getLatencySamples()<<" samples\n";check(elapsed<1,"Measured throughput exceeds realtime on this machine");}
 DiamondProcessor bypassed;set(bypassed,"harmonic13",100);set(bypassed,"bypass",1);bypassed.prepareToPlay(48000,128);auto dry=render(bypassed,48000,100,48000);double err=0;for(int j=bypassed.getLatencySamples();j<48000;++j)err=std::max(err,std::abs((double)dry[j]-(float)std::cos(juce::MathConstants<double>::twoPi*100*(j-bypassed.getLatencySamples())/48000)));check(err<1e-7,"Bypass remains exact delayed dry signal");
 snapshots(restored,folder);
 std::cout<<"FAILURES "<<failures<<"\n";return failures==0?0:1;
}
