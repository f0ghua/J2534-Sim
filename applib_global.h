#ifndef APPLIB_GLOBAL_H
#define APPLIB_GLOBAL_H

#if defined(APP_LIBRARY)
#define LIBSHARED_EXPORT __declspec(dllexport)
#else
#define LIBSHARED_EXPORT
#endif

#endif // APPLIB_GLOBAL_H
