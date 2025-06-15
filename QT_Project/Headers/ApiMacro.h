#ifdef APP_USE_DLL
#ifdef APP_BUILDING_PROJECT
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif
#else
#define API
#endif
