// ImgToArray.cpp
// NDK node that reads scanlines and stores them in a shared pixel buffer.
// Python then reads the buffer and sends it to ComfyUI.
//
// Build command (Linux, adjust paths to your Nuke installation):
//   g++ -shared -fPIC -O2 \
//       -I/usr/local/Nuke15.1v1/include \
//       -L/usr/local/Nuke15.1v1 \
//       -lDDImage \
//       -o pixelmapper.so pixelmapper.cpp
//
// Drop the .so into ~/.nuke/ or a folder on NUKE_PATH.

#include "DDImage/Iop.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs.h"
#include "DDImage/NukeWrapper.h"
#include "DDImage/Thread.h"

#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdint>

using namespace DD::Image;

// ─── Shared pixel buffer ──────────────────────────────────────────────────────
// Accessed from Python via nuke.toNode(...).knob('pixel_data_ptr').getValue()
// (see the Python side for how we pass the raw pointer).
//
// We store RGBA float64 (double), row-major, top→bottom.
// Width, height, and channel count are stored alongside the buffer so Python
// can reconstruct the correct NumPy shape without guessing.

struct PixelStore {
    std::vector<double> data;   // flat RGBA float64 array
    int width  = 0;
    int height = 0;
    int channels = 4;          // always RGBA; extend if you need more
    Lock lock;                 // DDImage thread lock
};

// One store per node instance (allocated on the heap, freed in destructor).
static PixelStore* g_store = nullptr;   // simplistic single-node example

// ─── Node class ───────────────────────────────────────────────────────────────

class ImgToArray : public Iop {
    PixelStore* _store;
    // UI knob to expose the raw pointer value so Python can read it.
    // (A Double_knob large enough to hold a 64-bit pointer.)
    double _ptrKnob;

public:
    ImgToArray(Node* node) : Iop(node), _store(new PixelStore()), _ptrKnob(0.0) {}

    ~ImgToArray() override { delete _store; }

    // ── DDImage boilerplate ──────────────────────────────────────────────────

    const char* Class()       const override { return "ImgToArray"; }
    const char* node_help()   const override {
        return "Buffers all scanlines into a float64 RGBA array that Python "
               "can convert to a NumPy array and forward to a ComfyUI API.";
    }

    void knobs(Knob_Callback f) override {
        // Expose the raw pointer as a hidden double knob.
        // Python reads it with:  node['pixel_data_ptr'].value()
        Double_knob(f, &_ptrKnob, "pixel_data_ptr", "Pixel Data Ptr");
        SetFlags(f, Knob::INVISIBLE | Knob::NO_ANIMATION | Knob::NO_RERENDER);

        Divider(f);
        // Add any processing controls here.
    }

    // ── Image processing ────────────────────────────────────────────────────

    void _validate(bool for_real) override {
        copy_info();    // pass format/channels through unchanged
        info_.black_outside(true);

        // Allocate / resize the pixel buffer now that we know the frame size.
        const Format& fmt = info_.format();
        int w = fmt.width();
        int h = fmt.height();

        Guard guard(_store->lock);
        _store->width    = w;
        _store->height   = h;
        _store->channels = 4;    // RGBA
        _store->data.assign(static_cast<size_t>(w) * h * 4, 0.0);

        // Publish the raw pointer through the knob so Python can find it.
        _ptrKnob = static_cast<double>(reinterpret_cast<uintptr_t>(_store));
    }

    void engine(int y, int x, int r, ChannelMask channels, Row& row) override {
        // Fetch RGBA from the input.
        input0().get(y, x, r, channels, row);

        const Format& fmt = info_.format();
        int width = fmt.width();

        // Map scanline y to a row index (Nuke scanlines are bottom-up).
        int rowIdx = (_store->height - 1) - y;
        if (rowIdx < 0 || rowIdx >= _store->height) return;

        const float* rPtr = row[Chan_Red]   + x;
        const float* gPtr = row[Chan_Green] + x;
        const float* bPtr = row[Chan_Blue]  + x;
        const float* aPtr = row[Chan_Alpha] + x;

        int numPix = r - x;
        double* dst = _store->data.data() + rowIdx * width * 4 + x * 4;

        for (int i = 0; i < numPix; ++i) {
            dst[i * 4 + 0] = rPtr[i];
            dst[i * 4 + 1] = gPtr[i];
            dst[i * 4 + 2] = bPtr[i];
            dst[i * 4 + 3] = aPtr[i];
        }
    }

    static const Iop::Description d;
};

// ─── Registration ─────────────────────────────────────────────────────────────

static Iop* build(Node* node) { return new NukeWrapper(new ImgToArray(node)); }

const Iop::Description ImgToArray::d("ImgToArray", "Image/ImgToArray", build);