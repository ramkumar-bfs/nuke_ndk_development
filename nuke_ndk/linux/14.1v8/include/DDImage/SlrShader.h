// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_SHADER_H
#define SLR_SHADER_H

#include "DDImage/Description.h"
#include "DDImage/Hash.h"
#include "DDImage/Pixel.h"             // convenience for SlrShader plugins

#include "DDImage/SlrRenderContext.h"  // convenience for SlrShader plugins
#include "DDImage/SlrShadingContext.h" // convenience for SlrShader plugins

#include "fdk/base/Hash.h"
#include "fdk/math/Mat4.h"

#include "usg/geom/ShaderDesc.h"
#include "usg/geom/Prim.h"
#include "usg/geom/LightPrim.h"

#include <unordered_map>

namespace DD {
  namespace Image {
    class Knob;
    class ViewerContext;
  }
}

namespace slr
{
  class SlrRenderContext;
  class SlrLightInfo;

  class SlrCoShader;
  class SlrSurfaceShader;
  class SlrGeometryShader;
  class SlrLightShader;
  class SlrVolumeShader;
  class SlrLensShader;

  //--------------------------------------------------


  // TODO: these helper function are here temporarily to help convert an IntArray or Int
  // usg::ShaderProperty into DD::Image::ChannelSet or DD::Image::Channel

DDImage_API
DD::Image::Channel getInputIntValueAsChannel(const usg::ShaderDesc&    shader_desc,
                                             const char*               prop_name,
                                             const DD::Image::Channel& def_chan = DD::Image::Chan_Black);

DDImage_API
void getInputIntArrayValueAsChannelArray(const usg::ShaderDesc& shader_desc,
                                         const char*            prop_name,
                                         std::vector<DD::Image::Channel>&  out,
                                         int32_t                default_size = 0);

DDImage_API
DD::Image::ChannelSet getInputIntArrayValueAsChannelSet(const usg::ShaderDesc&       shader_desc,
                                                        const char*                  prop_name,
                                                        const DD::Image::ChannelSet& def_channels = DD::Image::Mask_None);


  //--------------------------------------------------

  // Convenience for building OutputPortList functions
  #define SLRSAMPLEFUNC(A) reinterpret_cast<void*>(A)


  /*! \class DD::Image::SlrShader

    ScanlineRender-compatible shader class intended to be created
    through a plugin Description from an abstract shader description.

    The SlrSurfaceShader and SlrLightShader subclasses include the basic surface
    and lighting shader methods that are expected to be called from shading code.

  */
  class DDImage_API SlrShader
  {
  public:
    /*! Limited data types for shader inputs.
       (we'll use Nuke nomenclature here for no good reason...this likely should change
        to use ndk Value::Type)
    */
    enum KnobType
    {
      STRING_KNOB,      //!< std::string
      //
      INT_KNOB,         //!< Also used for boolean
      FLOAT_KNOB,       //!<
      DOUBLE_KNOB,      //!<
      //
      VEC2_KNOB,        //!< fdk::Vec2f
      VEC3_KNOB,        //!< fdk::Vec3f
      VEC4_KNOB,        //!< fdk::Vec4f
      MAT4_KNOB,        //!< fdk::Mat4d
      //
      COLOR3_KNOB,      //!< fdk::Vec3f - rgb
      COLOR4_KNOB,      //!< fdk::Vec4f - rgba
      COLOR_R_KNOB,     //!< 1 float 'red'
      COLOR_G_KNOB,     //!< 1 float 'green'
      COLOR_B_KNOB,     //!< 1 float 'blue'
      COLOR_A_KNOB,     //!< 1 float 'alpha'
      //
      INTARRAY_KNOB,    //!< Array of int32s
      FLOATARRAY_KNOB,  //!< Array of floats
      VEC2ARRAY_KNOB,   //!< Array of Vec2fs
      VEC3ARRAY_KNOB,   //!< Array of Vec3fs
      VEC4ARRAY_KNOB,   //!< Array of Vec4fs
      //
      PIXEL_KNOB,       //!< Pixel (also contains a ChannelSet)
      //
      INVALID_KNOB,
      NUM_KNOB_TYPES = INVALID_KNOB
    };

    //! Return the string version of the type enum.
    static const char* typeString(KnobType type);

    //! Return the number of data elements in the type enum.
    static size_t numElements(KnobType type);

    //! Return the number of float elements in the type enum.
    static size_t numFloats(KnobType type);

    //! Get the color ChannelSet that's appropriate for the KnobType. Data types return Mask_None.
    static DD::Image::ChannelSet getKnobColorChannels(KnobType type,
                                                      bool     isOutput = false);

    //-----------------------------------------------------


    /*! Nuke-specific input mapping
    */
    struct InputKnob
    {
      const char* name;       //!< Pointer to the name string - no local copy is made!
      KnobType    type;       //!< Data type of knob
      bool        isUniform;  //!< Hint that knob should not vary

      InputKnob(const char* _name,
                KnobType    _type,
                bool        is_uniform = false) :
        name((_name) ? _name : ""),
        type(_type),
        isUniform(is_uniform)
      {
        //
      }

      friend std::ostream& operator << (std::ostream& o, const InputKnob& b)
      {
        o << b.name << "(" << ((b.isUniform) ? "uniform " : "") << typeString(b.type) << ")";
        return o;
      }

    };

    using InputKnobList = std::vector<InputKnob>;

    //-----------------------------------------------------

    /*! Nuke-specific output mapping
    */
    struct OutputPort
    {
      const char*           name;           //!< Pointer to the name string - no local copy is made!
      KnobType              type;           //!< Type of port
      void*                 samplerFunc;    //!< Sampler function, cast to correct class using 'type'.
      DD::Image::ChannelSet colorChannels;  //!< Port's output color channels (cached result from validateShader())
      bool                  haveRgb;        //!< Does port provide Chans RGB? (cached result from validateShader())
      bool                  haveAlpha;      //!< Does port provide Chan_Alpha? (cached result from validateShader())

      OutputPort(const char* _name,
                 KnobType    _type,
                 void*       sampler_func) :
        name(_name),
        type(_type),
        samplerFunc(sampler_func),
        haveRgb(false),
        haveAlpha(false)
      {
          //
      }


      friend std::ostream& operator << (std::ostream& o, const OutputPort& b)
      {
        o << b.name << "(" << typeString(b.type) << ")";
        if (b.colorChannels != DD::Image::Mask_None) {
          o << "[" << b.colorChannels << "]";
          if (b.haveRgb) { o << "^rgb"; }
          if (b.haveAlpha) { o << "^a"; }
        }
        return o;
      }
    };

    using OutputPortList = std::vector<OutputPort>;

    //-----------------------------------------------------


    /*!
    */
    struct Connection
    {
      SlrShader*  sourceShader;         //!
      int32_t     outputPortIndex;      //!

      bool isValid() const { return (sourceShader != nullptr && outputPortIndex != -1); }

      Connection() : sourceShader(nullptr), outputPortIndex(-1) {}


      friend std::ostream& operator << (std::ostream& o, const Connection& b)
      {
        if (b.sourceShader) {
          o << "'" << b.sourceShader->getPath() << "' port " << b.outputPortIndex;
        }
        else {
          o << "<null>";
        }
        return o;
      }
    };

    using ConnectionList = std::vector<Connection>;
    using ConnectionMap  = std::unordered_map<std::string, Connection>;


    //-----------------------------------------------------

    struct IntBinding;
    struct FloatBinding;
    struct DoubleBinding;
    struct Vec2fBinding;
    struct Vec3fBinding;
    struct Vec4fBinding;
    struct Mat4dBinding;
    struct Color4Binding;
    struct PixelBinding;
    //
    struct Dual2fBinding;
    struct Dual2dBinding;
    struct Dual2Vec2fBinding;
    struct Dual2Vec3fBinding;
    struct Dual2Vec4fBinding;


    /*
        Output sampler handlers, typically assigned in the OutputPortList
    */
    typedef int32_t    (*IntOutputSampler)(const IntBinding&  , const SlrShadingContext*);
    typedef float      (*FloatOutputSampler)(const FloatBinding&, const SlrShadingContext*);
    typedef double     (*DoubleOutputSampler)(const DoubleBinding&, const SlrShadingContext*);
    typedef fdk::Vec2f (*Vec2fOutputSampler)(const Vec2fBinding&, const SlrShadingContext*);
    typedef fdk::Vec3f (*Vec3fOutputSampler)(const Vec3fBinding&, const SlrShadingContext*);
    typedef fdk::Vec4f (*Vec4fOutputSampler)(const Vec4fBinding&, const SlrShadingContext*);
    typedef fdk::Mat4d (*Mat4dOutputSampler)(const Mat4dBinding&, const SlrShadingContext*);
    typedef fdk::Vec4f (*Color4OutputSampler)(const Color4Binding&, const SlrShadingContext*);
    typedef void       (*PixelOutputSampler)(const PixelBinding&, const SlrShadingContext*, DD::Image::Pixel& out);
    //
    typedef void (*Dual2fOutputSampler)(const Dual2fBinding&, const SlrShadingContext*, Dual2f& out);
    typedef void (*Dual2dOutputSampler)(const Dual2dBinding&, const SlrShadingContext*, Dual2d& out);
    typedef void (*Dual2Vec2fOutputSampler)(const Dual2Vec2fBinding&, const SlrShadingContext*, Dual2Vec2f& out);
    typedef void (*Dual2Vec3fOutputSampler)(const Dual2Vec3fBinding&, const SlrShadingContext*, Dual2Vec3f& out);
    typedef void (*Dual2Vec4fOutputSampler)(const Dual2Vec4fBinding&, const SlrShadingContext*, Dual2Vec4f& out);

    /*
        Dedicated output sampler handlers which return the binding's uniform value.
        Note that for uniformPixelSampler only the enabled channels on the Pixel are cleared.
    */
    static int32_t     uniformIntSampler(const IntBinding&, const SlrShadingContext*);
    static float       uniformFloatSampler(const FloatBinding&, const SlrShadingContext*);
    static double      uniformDoubleSampler(const DoubleBinding&, const SlrShadingContext*);
    static fdk::Vec2f  uniformVec2fSampler(const Vec2fBinding&, const SlrShadingContext*);
    static fdk::Vec3f  uniformVec3fSampler(const Vec3fBinding&, const SlrShadingContext*);
    static fdk::Vec4f  uniformVec4fSampler(const Vec4fBinding&, const SlrShadingContext*);
    static fdk::Mat4d  uniformMat4dSampler(const Mat4dBinding&, const SlrShadingContext*);
    static fdk::Vec4f  uniformColor4Sampler(const Color4Binding&, const SlrShadingContext*);
    static void        uniformPixelSampler(const PixelBinding&, const SlrShadingContext*, DD::Image::Pixel& out);
    //
    static void uniformDual2fSampler(const Dual2fBinding&, const SlrShadingContext*, Dual2f& out);
    static void uniformDual2dSampler(const Dual2dBinding&, const SlrShadingContext*, Dual2d& out);
    static void uniformDual2Vec2fSampler(const Dual2Vec2fBinding&, const SlrShadingContext*, Dual2Vec2f& out);
    static void uniformDual2Vec3fSampler(const Dual2Vec3fBinding&, const SlrShadingContext*, Dual2Vec3f& out);
    static void uniformDual2Vec4fSampler(const Dual2Vec4fBinding&, const SlrShadingContext*, Dual2Vec4f& out);

    //-----------------------------------------------------

    struct InputBinding
    {
      SlrShader*  shader = nullptr;
      OutputPort* port = nullptr;
    };

    using InputBindingList = std::vector<InputBinding*>;


    struct PixelBinding : public InputBinding
    {
      PixelOutputSampler samplerFunc = uniformPixelSampler;

      inline bool isBound() const { return (samplerFunc != uniformPixelSampler); }

      inline void get(const SlrShadingContext& stx, DD::Image::Pixel& out) const { samplerFunc(*this, &stx, out); }
    };

    struct IntBinding : public InputBinding
    {
      int32_t          uniformVal;
      IntOutputSampler samplerFunc = uniformIntSampler;

      IntBinding(int32_t v = 0) : uniformVal(v) {}

      inline bool isBound() const { return (samplerFunc != uniformIntSampler); }

      inline int32_t get(const SlrShadingContext& stx) const { return samplerFunc(*this, &stx); }
      inline int32_t get() const { return samplerFunc(*this, nullptr); }
      inline operator int32_t() const { return samplerFunc(*this, nullptr); }
    };

    struct FloatBinding : public InputBinding
    {
      float              uniformVal;
      FloatOutputSampler samplerFunc = uniformFloatSampler;
      PixelBinding       pixelBinding;                        //!< Used if bound output port is a PIXEL_KNOB type
      DD::Image::Channel sampleChan = DD::Image::Chan_Black;  //!< Channel to sample in the PIXEL_KNOB's values

      FloatBinding(float v = 0.0f) : uniformVal(v) {}

      inline bool isBound() const { return (samplerFunc != uniformFloatSampler); }

      inline float get(const SlrShadingContext& stx) const { return samplerFunc(*this, &stx); }
      inline float get() const { return samplerFunc(*this, nullptr); }
      inline operator float() const { return samplerFunc(*this, nullptr); }
    };

    struct DoubleBinding : public InputBinding
    {
      double              uniformVal;
      DoubleOutputSampler samplerFunc = uniformDoubleSampler;

      DoubleBinding(double v = 0.0) : uniformVal(v) {}

      inline bool isBound() const { return (samplerFunc != uniformDoubleSampler); }

      inline double get(const SlrShadingContext& stx) const { return samplerFunc(*this, &stx); }
      inline double get() const { return samplerFunc(*this, nullptr); }
      inline operator double() const { return samplerFunc(*this, nullptr); }
    };

    struct Vec2fBinding : public InputBinding
    {
      fdk::Vec2f         uniformVal;
      Vec2fOutputSampler samplerFunc = uniformVec2fSampler;

      Vec2fBinding(const fdk::Vec2f& v = fdk::Vec2f(0.0f)) : uniformVal(v) {}

      inline bool isBound() const { return (samplerFunc != uniformVec2fSampler); }

      inline fdk::Vec2f get(const SlrShadingContext& stx) const { return samplerFunc(*this, &stx); }
      inline fdk::Vec2f get() const { return samplerFunc(*this, nullptr); }
      inline operator fdk::Vec2f() const { return samplerFunc(*this, nullptr); }
    };

    struct Vec3fBinding : public InputBinding
    {
      fdk::Vec3f         uniformVal;
      Vec3fOutputSampler samplerFunc = uniformVec3fSampler;

      Vec3fBinding(const fdk::Vec3f& v = fdk::Vec3f(0.0f)) : uniformVal(v) {}

      inline bool isBound() const { return (samplerFunc != uniformVec3fSampler); }

      inline fdk::Vec3f get(const SlrShadingContext& stx) const { return samplerFunc(*this, &stx); }
      inline fdk::Vec3f get() const { return samplerFunc(*this, nullptr); }
      inline operator fdk::Vec3f() const { return samplerFunc(*this, nullptr); }
    };

    struct Vec4fBinding : public InputBinding
    {
      fdk::Vec4f         uniformVal;
      Vec4fOutputSampler samplerFunc = uniformVec4fSampler;

      Vec4fBinding(const fdk::Vec4f& v = fdk::Vec4f(0.0f)) : uniformVal(v) {}

      inline bool isBound() const { return (samplerFunc != uniformVec4fSampler); }

      inline fdk::Vec4f get(const SlrShadingContext& stx) const { return samplerFunc(*this, &stx); }
      inline fdk::Vec4f get() const { return samplerFunc(*this, nullptr); }
      inline operator fdk::Vec4f() const { return samplerFunc(*this, nullptr); }
    };

    struct Mat4dBinding : public InputBinding
    {
      fdk::Mat4d         uniformVal;
      Mat4dOutputSampler samplerFunc = uniformMat4dSampler;

      inline bool isBound() const { return (samplerFunc != uniformMat4dSampler); }

      inline fdk::Mat4d get(const SlrShadingContext& stx) const { return samplerFunc(*this, &stx); }
      inline fdk::Mat4d get() const { return samplerFunc(*this, nullptr); }
      inline operator fdk::Mat4d() const { return samplerFunc(*this, nullptr); }
    };

    struct Color4Binding : public InputBinding
    {
      OutputPort*         opacityPort;      //!< Used if shared with color port
      fdk::Vec4f          uniformVal;
      Color4OutputSampler samplerFunc;      //!< Used if color & opacity are coming from same shader
      Vec3fBinding        colorBinding;     //!< Used if color & opacity are split
      FloatBinding        opacityBinding;   //!< Used if color & opacity are split

      bool isBound() const;

      fdk::Vec4f get(const SlrShadingContext& stx) const;
      fdk::Vec4f get() const;
      operator fdk::Vec4f() const;

      Color4Binding() : opacityPort(nullptr), samplerFunc(uniformColor4Sampler) {}
      Color4Binding(const fdk::Vec4f& v) :
        opacityPort(nullptr),
        samplerFunc(uniformColor4Sampler),
        colorBinding(v.xyz()),
        opacityBinding(v.w) {}
    };

    struct Dual2fBinding : public FloatBinding
    {
      Dual2fOutputSampler dual2SamplerFunc = uniformDual2fSampler;

      Dual2fBinding(float v = 0.0) : FloatBinding(v) {}

      inline bool isBound() const { return (dual2SamplerFunc != uniformDual2fSampler); }

      inline void get(const SlrShadingContext& stx, Dual2f& v) const { dual2SamplerFunc(*this, &stx, v); }
      inline void get(Dual2f& v) const { return dual2SamplerFunc(*this, nullptr, v); }
    };

    struct Dual2dBinding : public DoubleBinding
    {
      Dual2dOutputSampler dual2SamplerFunc = uniformDual2dSampler;

      Dual2dBinding(double v = 0.0) : DoubleBinding(v) {}

      inline bool isBound() const { return (dual2SamplerFunc != uniformDual2dSampler); }

      inline void get(const SlrShadingContext& stx, Dual2d& v) const { dual2SamplerFunc(*this, &stx, v); }
      inline void get(Dual2d& v) const { return dual2SamplerFunc(*this, nullptr, v); }
    };

    struct Dual2Vec2fBinding : public Vec2fBinding
    {
      Dual2Vec2fOutputSampler dual2SamplerFunc = uniformDual2Vec2fSampler;

      Dual2Vec2fBinding(const fdk::Vec2f& v = fdk::Vec2f(0.0f)) : Vec2fBinding(v) {}

      inline bool isBound() const { return (dual2SamplerFunc != uniformDual2Vec2fSampler); }

      inline void get(const SlrShadingContext& stx, Dual2Vec2f& v) const { dual2SamplerFunc(*this, &stx, v); }
      inline void get(Dual2Vec2f& v) const { return dual2SamplerFunc(*this, nullptr, v); }
    };

    struct Dual2Vec3fBinding : public Vec3fBinding
    {
      Dual2Vec3fOutputSampler dual2SamplerFunc = uniformDual2Vec3fSampler;

      Dual2Vec3fBinding(const fdk::Vec3f& v = fdk::Vec3f(0.0f)) : Vec3fBinding(v) {}

      inline bool isBound() const { return (dual2SamplerFunc != uniformDual2Vec3fSampler); }

      inline void get(const SlrShadingContext& stx, Dual2Vec3f& v) const { dual2SamplerFunc(*this, &stx, v); }
      inline void get(Dual2Vec3f& v) const { return dual2SamplerFunc(*this, nullptr, v); }
    };

    struct Dual2Vec4fBinding : public Vec4fBinding
    {
      Dual2Vec4fOutputSampler dual2SamplerFunc = uniformDual2Vec4fSampler;

      Dual2Vec4fBinding(const fdk::Vec4f& v = fdk::Vec4f(0.0f)) : Vec4fBinding(v) {}

      inline bool isBound() const { return (dual2SamplerFunc != uniformDual2Vec4fSampler); }

      inline void get(const SlrShadingContext& stx, Dual2Vec4f& v) const { dual2SamplerFunc(*this, &stx, v); }
      inline void get(Dual2Vec4f& v) const { return dual2SamplerFunc(*this, nullptr, v); }
    };

    //-----------------------------------------------------



  private:
    //! Disabled copy constructor.
    SlrShader(const SlrShader&) = delete;
    //! Disabled assignment operator.
    SlrShader& operator = (const SlrShader&) = delete;


  public:
    //-----------------------------------------------------------------
    /*! This structure creates a subclass of SlrShader, possibly
        from loading a plugin file or a compiled-in Description.
    */
    //-----------------------------------------------------------------
    class DDImage_API PluginDescription : public DD::Image::Description
    {
    private:
      const char* _shaderClass;       //!< Name of shader class from the ctor - *NOT* the plugin .so filename!

      //! Method type defined in Description.h: (*f)(Description*), called when the plugin .so is first loaded.
      static void pluginBuilderCallback(DD::Image::Description* desc);


    public:
      //! Constructor method definitions used for 'build()' methods in plugins.
      typedef SlrShader* (*PluginBuilderVoid)(void);
      typedef SlrShader* (*PluginBuilder)(DD::Image::Op* parent_op);
      PluginBuilderVoid   builderFunc;        // <<< Call this to construct a SlrShader object.


    public:
      //! Constructors set name and label to same value.
      PluginDescription(const char*    shader_class,
                        PluginBuilder  builder);
      PluginDescription(const char*       shader_class,
                        PluginBuilderVoid builder);

      /*! Prefix and suffix strings prepended and appended to shaderClass() string to
          construct plugin filenames (vs. the loaded Description name)
      */
      static const char*  pluginPrefix() { return "slr"; }
      static const char*  pluginSuffix() { return ""; }

      //! Name of the shader class without any leading pluginPrefix() or trailing pluginSuffix().
      const char*         shaderClass() const { return _shaderClass; }

      /*! Find a dso description by shader class name.

          If a Description already exists it's immediately returned otherwise pluginPrefix()
          and pluginSuffix() are added to the shaderClass() string to search for the
          plugin .so filename.

          Returns nullptr if not found.
      */
      static const PluginDescription* find(const char* shader_class);
    };
    //-----------------------------------------------------------------


    typedef std::unordered_map<std::string, uint32_t> KnobNameMap;


  protected:
    std::string           _path;                    //!< Shader path in material group
    bool                  _debug;                   //!< 
    int32_t               _updateVersion;           //!< Update version incremented in updateProperties()
    //
    InputKnobList         _inputs;                  //!< Input knobs, copied and updated from the static list
    KnobNameMap           _inputNameMap;            //!< Name to input index map
    OutputPortList        _outputs;                 //!< Output knobs, copied from the static list
    KnobNameMap           _outputNameMap;           //!< Name to output index map
    //
    ConnectionList        _inputConnections;        //!< Shaders bound to input knobs
    //
    DD::Image::ChannelSet _inputTextureChannels;    //!< Set of channels output by all input texture bindings that this shader needs
    DD::Image::ChannelSet _outputColorChannels;     //!< Set of all output color channels this shader produces
    DD::Image::ChannelSet _outputDataChannels;      //!< Set of all output data (non-color) channels this shader produces
    DD::Image::Format     _outputFormat;            //!< Nominal Format if the surface shader represents an image/texture
    //
    DD::Image::ChannelPack _outputColorChannelPack; //!< Automatically built from _outputColorChannels
    DD::Image::ChannelPack _outputDataChannelPack;  //!< Automatically built from _outputDataChannels

    static SlrShader* _create(const char* slr_shader_class);

    //! Default implementation combines the channels from all inputs.
    virtual void  _validateShader();

    //! Default implementation does nothing.
    virtual void  _requestTextures(const DD::Image::ChannelSet& request_channels,
                                   SlrRenderContext&            slrtx);

    //! Default implementation does nothing.
    virtual void  _initializeForShading(uint32_t          num_shading_threads,
                                        SlrRenderContext& slrtx) {}

    //! Default implementation does nothing.
    virtual void  _cleanupAfterShading() {}


  public:
    //!
    SlrShader(const char* path = "");
    //!
    SlrShader(const InputKnobList&  inputs,
              const OutputPortList& outputs,
              void*                 inputs_storage_data = nullptr,
              size_t                inputs_storage_size = 0);

    //! Must have a virtual destructor
    virtual ~SlrShader() {}

    //! Returns the class name, must implement.
    virtual const char*  shaderClass() const = 0;


    /*! Create a SlrShader instance from a shader class name, an existing shader description,
        or a ShaderDesc.
        Calling code takes ownership of returned pointer.
    */
    static SlrShader*  create(const PluginDescription& shader_description);
    static SlrShader*  create(const char* slr_shader_class);
    static SlrShader*  create(const usg::ShaderDesc* shader_desc);

    //! Find a SlrShader::PluginDescription by shader class name - *not* the plugin name!
    static const PluginDescription* find(const char* shader_class) { return PluginDescription::find(shader_class); }


    //!
    virtual SlrSurfaceShader*  asSurfaceShader()  { return nullptr; }
    //!
    virtual SlrCoShader*       asCoShader()       { return nullptr; }
    //!
    virtual SlrGeometryShader* asGeometryShader() { return nullptr; }
    //!
    virtual SlrLightShader*    asLightShader()    { return nullptr; }
    //!
    virtual SlrVolumeShader*   asVolumeShader()   { return nullptr; }
    //!
    virtual SlrLensShader*     asLensShader()     { return nullptr; }


  public:
    //---------------------------------------------------------
    //---------------------------------------------------------

    //! Return the shader path if assigned. Used mainly for debugging.
    const std::string&  getPath() const { return _path; }
    //! Assign a shader path. Used mainly for debugging.
    void                setPath(const std::string& path) { _path = path; }

    //!
    int32_t  getUpdateVersion() const { return _updateVersion; }
    //!
    void     setUpdateVersion(int32_t v) { _updateVersion = v; }


    //! Return a static list of input knobs for this shader. Base class returns an empty list.
    virtual const InputKnobList&   getInputKnobDefinitions() const;

    //! Return a static list of output knobs for this shader. Base class returns only the 'primary' output.
    virtual const OutputPortList&  getOutputPortDefinitions() const;


    //---------------------------------------------------------
    //
    //---------------------------------------------------------


    //! Returns the number of input knobs.
    uint32_t     numInputs()  const { return (uint32_t)_inputs.size();  }
    //! Returns the number of output knobs.
    uint32_t     numOutputs() const { return (uint32_t)_outputs.size(); }

    //! Get the list of input connections.
    const ConnectionList&  inputConnections() const { return _inputConnections; }

    //! Returns true if the InputKnob type and OutputPort type are compatible.
    static bool  isConnectionTypeCompatible(const InputKnob&  input_knob,
                                            const OutputPort& output_port);

    //!
    bool         connectInput(const std::string& input_name,
                              SlrShader*         source_shader,
                              int32_t            output_port_index);
    bool         connectInput(const std::string& input_name,
                              SlrShader*         source_shader,
                              const std::string& output_port_name);
    bool         connectInput(const std::string& input_name,
                              const Connection&  connection);
    //!
    bool         connectInputs(const ConnectionMap& connections);


    //! Return the connection for the named input, otherwise an invalid one is returned.
    const Connection&  getConnection(const std::string& input_name) const;

    /*! Convenience to get a SlrShader input.
        Same as calling 'getConnection(input_name)->source_shader'.
    */
    SlrShader*         getShaderForInput(const std::string& input_name) const;


    //---------------------------------------------------------


    //! Returns input knob or null if not available.
    InputKnob*   getInputKnob(uint32_t input) const;
    InputKnob*   getInputKnob(const std::string& input_name) const;

    //! Return a named input's index or -1 if not found.
    int32_t      getInputIndex(const std::string& input_name) const;


    //! Returns output knob or null if not available.
    OutputPort*  getOutputPort(uint32_t output) const;
    OutputPort*  getOutputPort(const std::string& output_name) const;

    //! Return a named output's index or -1 if not found.
    int32_t      getOutputIndex(const std::string& output_name) const;



    /*! Initialize an input binding by retrieving the named input property from the
        passed-in ShaderDesc and assigning the appropriate sampler function.

        If the ShaderDesc has the named input property then its default value
        is copied to the binding's 'uniformVal'. If the input property is connected
        to another shader the binding's 'samplerFunc' is set to the other shader's
        OutputPort 'samplerFunc' pointer. If the pointer is null then the uniform
        sampler for this type is assigned which will return the binding's
        'uniformVal'.

        If the ShaderDesc does not have the named input property the passed-in
        'fallback' value is copied to the binding's 'uniformVal' and the sampler
        set to the uniform sampler func for this type.
    */
    IntBinding        bindIntShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, int32_t fallback = 0) const;
    DoubleBinding  bindDoubleShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, double  fallback = 0.0) const;
    //
    FloatBinding    bindFloatShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, float fallback = 0.0f) const;
    FloatBinding    bindFloatShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, DD::Image::Channel sample_chan, float fallback = 0.0f) const;
    //
    Vec2fBinding    bindVec2fShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, const fdk::Vec2f& fallback = fdk::Vec2f(0.0f)) const;
    Vec3fBinding    bindVec3fShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, const fdk::Vec3f& fallback = fdk::Vec3f(0.0f)) const;
    Vec4fBinding    bindVec4fShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, const fdk::Vec4f& fallback = fdk::Vec4f(0.0f)) const;
    //
    Mat4dBinding    bindMat4dShaderDescInput(const usg::ShaderDesc&, const std::string& input_name, const fdk::Mat4d& fallback = fdk::Mat4d::getIdentity()) const;
    //
    //! Specialized double bind for color and opacity coming from the same source shader, a very common case.
    Color4Binding   bindColor4ShaderDescInput(const usg::ShaderDesc& shader_desc,
                                              const std::string&     color_input_name,
                                              const std::string&     opacity_input_name,
                                              const fdk::Vec4f&      fallback = fdk::Vec4f(1.0f)) const;
    //!
    PixelBinding    bindPixelShaderDescInput(const usg::ShaderDesc&, const std::string& input_name) const;

    /*! Create a secondary Dual2 binding to a valid, existing binding. Requires the name of the
        Dual2 output port on the source shader. Typically if an output is named 'result' then
        the Dual2 version of that output is named 'resultDual2'.
    */
    Dual2fBinding     bindDual2fInput(const FloatBinding&, const std::string& dual2_output_name) const;
    Dual2dBinding     bindDual2dInput(const DoubleBinding&, const std::string& dual2_output_name) const;
    Dual2Vec2fBinding bindDual2Vec2fInput(const Vec2fBinding& in, const std::string& dual2_output_name) const;
    Dual2Vec3fBinding bindDual2Vec3fInput(const Vec3fBinding& in, const std::string& dual2_output_name) const;
    Dual2Vec4fBinding bindDual2Vec4fInput(const Vec4fBinding& in, const std::string& dual2_output_name) const;


    /*! Update an input binding's sampler function if there's a valid connection on the
        named input. Returns true if there's a valid connection to another shader and
        the binding is bound to it.

        Retrieves and assigns the correct value sampler to use depending if the InputKnob
        is connected to another shader's OutputPort. If the input is not connected or the
        connected OutputPort's 'samplerFunc' pointer is null then the uniform sampler
        is assigned which returns the binding's 'uniformVal' var, and false is returned
        from this method.

        The binding's 'uniformVal' can be directly set or normally it's initialized using
        the 'bind<type>ShaderDescInput()' method which accepts an optional fallback value
        if the ShaderDesc does not have that property.
    */
    bool     connectIntInputBinding(IntBinding&,    const std::string& input_name) const;
    bool   connectFloatInputBinding(FloatBinding&,  const std::string& input_name, DD::Image::Channel sample_chan = DD::Image::Chan_Black) const;
    bool  connectDoubleInputBinding(DoubleBinding&, const std::string& input_name) const;
    bool   connectVec2fInputBinding(Vec2fBinding&,  const std::string& input_name) const;
    bool   connectVec3fInputBinding(Vec3fBinding&,  const std::string& input_name) const;
    bool   connectVec4fInputBinding(Vec4fBinding&,  const std::string& input_name) const;
    bool   connectMat4dInputBinding(Mat4dBinding&,  const std::string& input_name) const;
    bool   connectPixelInputBinding(PixelBinding&,  const std::string& input_name) const;


    /*! Return the channels output by the output port.

        By default the port's type is used to determine the channels. Only types that are
        explicitly color types return a valid channel set. So COLOR3_KNOB returns Mask_RGB
        while COLOR4_KNOB returns Mask_RGBA, and COLOR_R_KNOB returns Mask_Red, COLOR_A_KNOB
        returns Mask_Alpha, etc.

        Single value types like INT, FLOAT and DOUBLE, and vector/matrix types all
        return Mask_None as they are general data, and not explicitly 'colors'.

        PIXEL is handled special - if the output port name is 'surface' the SlrShader's
        output ChannelSet is returned, otherwise Mask_None is returned. A shader subclass
        should override this method to correctly define the set in that case.

        Output types which don't naturally translate to a fixed set of Channels return
        Mask_None, such as STRING, MAT4, INTARRAY, FLOATARRAY, DOUBLEARRAY, etc.
    */
    virtual DD::Image::ChannelSet  getOutputPortChannels(const OutputPort& output_port) const;


  public:
    //---------------------------------------------------------
    //---------------------------------------------------------


    /*! Copy/update property values from a usg::ShaderDesc shader schema source.
        Must implement.
    */
    virtual void  updateProperties(const usg::ShaderDesc& shader_desc) = 0;

    /*! Initialize any vars prior to rendering. This is called single-threaded
        by the Renderer during Iop::_validate().

        A typical SlrShader will take input variables and do any costly pre-calcs
        here to save on per-shade cost. Don't bother storing anything that may
        change on a per-sample basis!

        Base class calls validateShader() on the set of input shaders then
        calls the virtual _validateShader() for the subclass to do its work.
    */
    void  validateShader();


    /*! Request the texture-Iop samplers this shader will want to use during shading
        when input DD::Image::Iop sources need to be directly sampled.

        This is called single-threaded by the Renderer during Iop::_request(),
        in preparation for the threaded Iop engine() calls when shading is done.

        The shader requests an Iop sampler by calling slrtx.requestTextureSampler()
        and passing the input Iop* and the channels that will be read from it.
        Since there can be multiple shaders accessing the same source Iop*
        with different channel sets, all the shader requests are collated together
        before the DD::Image::Sampler objects are created by the Renderer, so
        the created Sampler will contain the union of all requested channels.

        To complete the request process the shader should retrieve the created
        DD::Image::Samplers in initializeForShading() using
        slrtx.getTextureSampler(Iop*), and store them locally so they're
        available during the threaded shading methods.

        Base class calls requestTextures() on the set of input shaders then
        calls the virtual _requestTextures() for the subclass to do its work.
    */
    void  requestTextures(const DD::Image::ChannelSet& request_channels,
                          SlrRenderContext&            slrtx);

    /*! Initialize the shader's state in preparation for multi-threaded shading,
        called by the Renderer in the first engine() call so the total number
        of shading threads is known at the time this method runs.

        Any Iop texture samplers requested in requestShaders() have been created
        by the Renderer and the shader can make local copies of the
        DD::Image::Sampler pointers by querying slrtx.getTextureSampler(iop).

        The maximum(total) number of Renderer engine threads that will ever be
        used during shading is provided so the shader can allocate any local
        storage used by each unique shading thread.

        The SlrShadingContext passed to the shader methods contains the current
        thread index so the shader can access the correct local storage.

        Base class calls initializeForShading() on the set of input shaders then
        calls the virtual _initializeForShading() for the subclass to do its work.
    */
    void  initializeForShading(uint32_t          num_shading_threads,
                               SlrRenderContext& slrtx);


    /*! Called when the render is complete and the shader can delete any temp
        allocations done in initializeForShading().
        The Renderer calls this on Iop::_close() which clears out render prims,
        texture samplers, etc. The Renderer will do another
        validate/request/initialize sequence before the shaders are called
        again to shade anything.
 
        Base class calls cleanupAfterShading() on the set of input shaders then
        calls the virtual _cleanupAfterShading() for the subclass to do its work.
    */
    void  cleanupAfterShading();


    /*! Return the channels for all the texture sources this shader intends to
        use.
        This ChannelSet is built in validateShader() *after* the subclass
        _validateShader() has been called and all bindings connected.
    */
    const DD::Image::ChannelSet&   inputTextureChannels() const { return _inputTextureChannels; }

    /*! Return the color channels (including alphas and masks) output by this shader.
        This ChannelSet should be constructed in validateShader().
    */
    const DD::Image::ChannelSet&   outputColorChannels() const { return _outputColorChannels; }
    //! Fast-access ChannelPack version of outputColorChannels() for use in varying shader code.
    const DD::Image::ChannelPack&  outputColorChannelPack() const { return _outputColorChannelPack; }

    /*! Return the data (non-color) channels output by this shader such as normals, IDs, etc.
        This ChannelSet should be constructed in validateShader().
    */
    const DD::Image::ChannelSet&   outputDataChannels() const { return _outputDataChannels; }
    //! Fast-access ChannelPack version of outputDataChannels() for use in varying shader code.
    const DD::Image::ChannelPack&  outputDataChannelPack() const { return _outputDataChannelPack; }

    //! Return the nominal Format if the surface shader represents an image/texture.
    const DD::Image::Format&       outputFormat() const { return _outputFormat; }


  public:
    //!
    void print(const char*   prefix,
               std::ostream& o) const;
    friend std::ostream& operator << (std::ostream& o, const SlrShader& b) { b.print("", o); return o; }


  }; // SlrShader

  using SlrShaderList = std::vector<SlrShader*>;
  using SlrShaderMap  = std::unordered_map<std::string, SlrShader*>;


  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------


  /*! A list of SlrShader pointers and the group's output.

      The pointers are *not* managed by this class but there's convenience
      methods to help delete them and clear out the group.
  */
  template <class ShaderT>
  struct SlrShaderGroup
  {
    SlrShaderMap shaders;       //!< Shaders with name mapping for connections
    ShaderT*     output;        //!< Shader graph entry point
    //
    fdk::Hash  topologyHash;    //!< Does the shader graph need to be rebuilt?
    fdk::Hash  propertyHash;    //!< Do the shaders need parameter updating?


    SlrShaderGroup();

    size_t  size() const { return shaders.size(); }
    bool    empty() const { return shaders.empty(); }

    //! Return the named shader, or null if not in group.
    SlrShader*  getShaderNode(const std::string& shader_node_name) const;

    //! Convenience function to delete shader pointers and empty the group.
    void        deleteShaders();

  };


  // Predeclare the supported shader group specializations:
  using SlrCoShaderGroup       = SlrShaderGroup<SlrCoShader>;
  using SlrSurfaceShaderGroup  = SlrShaderGroup<SlrSurfaceShader>;
  using SlrGeometryShaderGroup = SlrShaderGroup<SlrGeometryShader>;
  using SlrLightShaderGroup    = SlrShaderGroup<SlrLightShader>;
  using SlrVolumeShaderGroup   = SlrShaderGroup<SlrVolumeShader>;
  using SlrLensShaderGroup     = SlrShaderGroup<SlrLensShader>;


//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------


template <class ShaderT>
inline
SlrShaderGroup<ShaderT>::SlrShaderGroup() :
  output(nullptr)
{
  //
}

template <class ShaderT>
inline SlrShader*
SlrShaderGroup<ShaderT>::getShaderNode(const std::string& shader_node_name) const
{
  const typename SlrShaderMap::const_iterator it = shaders.find(shader_node_name);
  return (it != shaders.end()) ? it->second : nullptr;
}

template <class ShaderT>
inline void
SlrShaderGroup<ShaderT>::deleteShaders()
{
  for (typename std::unordered_map<std::string, SlrShader*>::iterator it=shaders.begin(); it != shaders.end(); ++it)
  {
    delete it->second;
  }
  shaders.clear();
  output = nullptr;
  topologyHash.reset();
  propertyHash.reset();
}

//----------------------------------------------------

inline bool
SlrShader::Color4Binding::isBound() const
{
  if (shader) {
    return (samplerFunc != uniformColor4Sampler); // color4 takes precedence
  }
  return (colorBinding.isBound() && opacityBinding.isBound());
}

inline fdk::Vec4f
SlrShader::Color4Binding::get(const SlrShadingContext& stx) const {
  if (shader) {
    return samplerFunc(*this, &stx); // color4 takes precedence
  }
  return fdk::Vec4f(colorBinding.samplerFunc(colorBinding, &stx),
                    opacityBinding.samplerFunc(opacityBinding, &stx));
}

inline fdk::Vec4f
SlrShader::Color4Binding::get() const {
  if (shader) {
    return samplerFunc(*this, nullptr); // color4 takes precedence
  }
  return fdk::Vec4f(colorBinding.samplerFunc(colorBinding, nullptr),
                    opacityBinding.samplerFunc(opacityBinding, nullptr));
}

inline SlrShader::Color4Binding::operator fdk::Vec4f() const
{
  return this->get();
}


} // namespace slr


#endif
