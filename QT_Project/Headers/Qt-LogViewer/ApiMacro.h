#ifdef QT_LOGVIEWER_USE_DLL
#ifdef QT_LOGVIEWER_BUILDING_PROJECT
#define QT_LOGVIEWER_API __declspec(dllexport)
#else
#define QT_LOGVIEWER_API __declspec(dllimport)
#endif
#else
#define QT_LOGVIEWER_API
#endif
