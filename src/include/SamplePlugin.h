// Headers
#pragma once
#ifndef SAMPLEPLUGIN_H
#define SAMPLEPLUGIN_H

#include "DDImage/Iop.h"
#include "DDImage/description.h"


class SamplePlugin : public DD::Image::Iop{
    // Defining class public members
public:
    // Constructor
    explicit SamplePlugin(Node* node);
    // Static mmebers for the plugin
    // Static description for the plugin
    static const DD::Image::Iop::Description description;
    static DD::Image::Iop* build(Node* node);

    const char* Class() const override ;
    const char* node_help() const override;
    
    // Node paramater Knobs
    void knobs(DD::Image::Knob_Callback f) override;
    // Node Output format and metadata
    void _validate(bool for_real) override;
    // Node Output pixel processing
    void engine(int y, int x, int r ,DD::Image::ChannelMask channels, DD::Image::Row& row) override;
    // Destructor
    ~SamplePlugin();

};

#endif // SAMPLEPLUGIN_H