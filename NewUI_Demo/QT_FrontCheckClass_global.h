#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(QT_FrontCheckClass_LIB)
#  define QT_FrontCheckClass_EXPORT Q_DECL_EXPORT
# else
#  define QT_FrontCheckClass_EXPORT Q_DECL_IMPORT
# endif
#else 
# define QT_FrontCheckClass_EXPORT
#endif
