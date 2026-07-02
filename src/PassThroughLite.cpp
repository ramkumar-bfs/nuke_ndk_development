#include "DDImage/Iop.h"
#include "DDImage/Row.h"

using namespace DD::Image;

class PassThroughLite : public Iop {
public:
    explicit PassThroughLite(Node* node) : Iop(node) {}

    const char* Class() const override { return "PassThroughLite"; }
    const char* node_help() const override {
        return "Minimal custom NDK node that forwards pixels from input to output.";
    }

    int minimum_inputs() const override { return 1; }
    int maximum_inputs() const override { return 1; }

    void _validate(bool for_real) override {
        copy_info();
        Iop::_validate(for_real);
    }

    void _request(int x, int y, int r, int t, ChannelMask channels, int count) override {
        input0().request(x, y, r, t, channels, count);
    }

    void engine(int y, int x, int r, ChannelMask channels, Row& row) override {
        input0().get(y, x, r, channels, row);
    }

    static Iop* build(Node* node) {
        return new PassThroughLite(node);
    }

    static const Iop::Description d;
};

const Iop::Description PassThroughLite::d("Filter/PassThroughLite", PassThroughLite::build);
