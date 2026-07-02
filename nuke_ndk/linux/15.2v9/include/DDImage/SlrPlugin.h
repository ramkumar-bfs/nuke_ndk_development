// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_PLUGIN_H
#define SLR_PLUGIN_H

#include "DDImage/Description.h"

namespace slr
{

  static constexpr const char* pluginPrefix = "slr";


  /*! \class DD::Image::SlrPlugin
      Abstract base class for Slr plugins (shaders, integrators, etc)
  */
  class DDImage_API SlrPlugin
  {
  public:
    //-----------------------------------------------------------------
    /*! This structure creates a subclass of an Slr plugin, possibly
        from loading a plugin file or a compiled-in Description.
    */
    //-----------------------------------------------------------------
    class DDImage_API Description : public DD::Image::Description
    {
    protected:
      const char* _pluginClass;   //!< Name of plugin class from the ctor - *NOT* the plugin .so filename!

      //! Method type defined in Description.h: (*f)(Description*), called when the plugin .so
      //! is first loaded. This adds the plugin class to the map of loaded dsos so that we don't
      //! need to search or load the .so again.
      static void  pluginBuilderCallback(DD::Image::Description* desc);


    public:
      //! Constructor method definitions used for 'build()' methods in plugins.
      typedef SlrPlugin* (*PluginBuilder)(void);
      PluginBuilder   builderFunc;        // <<< Call this to construct a SlrPlugin object.


    public:
      //! Constructors set name and label to same value.
      Description(const char*   className,
                  PluginBuilder builder);

      //! Name of the plugin class without any leading pluginPrefix() or trailing pluginSuffix().
      const char*          pluginClass() const { return _pluginClass; }


      /*! Find a dso description by name.

          If it has been previously been loaded this method quickly returns an existing cached
          SlrPlugin::Description, otherwise it prepends SlrPlugin::pluginPrefix() to the start
          of the name and SlrPlugin::pluginSuffix() to the end of the name
          (ie 'slrMyCustomIntegrator') before searching the plugin paths for a matching plugin
          filename.

          Returns nullptr if not found.
      */
      static const Description*  find(const char* className,
                                      const char* pluginPrefix = slr::pluginPrefix,
                                      const char* pluginSuffix = "");
    };


  protected:
    //! Subclasses call this with their custom prefix & suffix.
    static SlrPlugin*  _create(const char* className,
                               const char* pluginPrefix,
                               const char* pluginSuffix);


  public:
    virtual ~SlrPlugin() {}

  };


} // namespace slr


#endif
