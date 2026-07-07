// DCC Includes
#include <DDImage/Iop.h>

// Local includes
#include "include/SamplePlugin.h"

// namespace
using namespace DD::Image;

static const char kSamplePluginClass[] = "SamplePlugin";

SamplePlugin::SamplePlugin(Node* node) : DD::Image::Iop(node) {
    // Constructor implementation 
}

const char* SamplePlugin::Class() const {
    return kSamplePluginClass;
}

const char* SamplePlugin::node_help() const {
    return "This is a sample plugin Node.";
}

void SamplePlugin::knobs(Knob_Callback f) {
    Iop::knobs(f);
}

void SamplePlugin::_validate(bool for_real) {
    copy_info();
    Iop::_validate(for_real);
}

void SamplePlugin::engine(int y, int x, int r, ChannelMask channels, Row& row) {
    input0().get(y, x, r, channels, row);
}

Iop* SamplePlugin::build(Node *node){
    return new SamplePlugin(node);
}

SamplePlugin::~SamplePlugin() {
    // Destructor implementation
}


const Iop::Description SamplePlugin::description(kSamplePluginClass, "Demo/SamplePlugin",SamplePlugin::build);