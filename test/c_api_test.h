#ifndef C_API_TEST_H
#define C_API_TEST_H

#ifdef __cplusplus
#define __extern_c extern "C"
#else
#define __extern_c
#endif

__extern_c int c_app(int argc, const char *argv[]);



#endif // C_API_TEST_H
