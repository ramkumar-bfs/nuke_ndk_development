
#ifndef OPENASSETIO_CORE_EXPORT_H
#define OPENASSETIO_CORE_EXPORT_H

#ifdef OPENASSETIO_CORE_STATIC_DEFINE
#  define OPENASSETIO_CORE_EXPORT
#  define OPENASSETIO_CORE_NO_EXPORT
#else
#  ifndef OPENASSETIO_CORE_EXPORT
#    ifdef openassetio_core_EXPORTS
        /* We are building this library */
#      define OPENASSETIO_CORE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define OPENASSETIO_CORE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef OPENASSETIO_CORE_NO_EXPORT
#    define OPENASSETIO_CORE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef OPENASSETIO_CORE_DEPRECATED
#  define OPENASSETIO_CORE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef OPENASSETIO_CORE_DEPRECATED_EXPORT
#  define OPENASSETIO_CORE_DEPRECATED_EXPORT OPENASSETIO_CORE_EXPORT OPENASSETIO_CORE_DEPRECATED
#endif

#ifndef OPENASSETIO_CORE_DEPRECATED_NO_EXPORT
#  define OPENASSETIO_CORE_DEPRECATED_NO_EXPORT OPENASSETIO_CORE_NO_EXPORT OPENASSETIO_CORE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef OPENASSETIO_CORE_NO_DEPRECATED
#    define OPENASSETIO_CORE_NO_DEPRECATED
#  endif
#endif
#define OPENASSETIO_CORE_ABI_VERSION v1
#endif /* OPENASSETIO_CORE_EXPORT_H */
