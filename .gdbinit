# Workspace GDB defaults for Nuke NDK plugin debugging.
set pagination off
set confirm off
set breakpoint pending on
set print pretty on
set disassemble-next-line on

# Shared-library breakpoints can be deferred until plugins are loaded.
set stop-on-solib-events 1

# Common NDK plugin entry points in this workspace.
rbreak ^PassThroughLite::engine$
rbreak ^PassThroughLite::_validate$
rbreak ^PassThroughLite::_request$
rbreak ^PassThroughLite::build$

rbreak ^SamplePlugin::engine$
rbreak ^SamplePlugin::_validate$
rbreak ^SamplePlugin::build$

rbreak ^ImgToArray::engine$
rbreak ^ImgToArray::_validate$

# Node factory wrappers often called when Nuke instantiates a plugin node.
rbreak ^build$
