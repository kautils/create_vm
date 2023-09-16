
#ifndef INCLUDE_KAUTIL_JNI_JNI_H
#define INCLUDE_KAUTIL_JNI_JNI_H

#include <stdint.h>

struct JNIEnv_;
typedef JNIEnv_ JNIEnv;

namespace kautil{
namespace jni{

struct JVMInternal;
class JVM {
public:
    JVM();
    virtual ~JVM();
    virtual void Xmx(const char * value)const; 
    virtual void Xms(const char * value)const;
    virtual void Xss(const char * value)const;
    virtual void add_java_class_path(const char ** array,uint64_t length)const;
    virtual void add_java_class_path_with_directory(const char * directory)const;
    virtual void add_java_class_path(const char * p)const;
    virtual long jni_version()const;
    virtual void create_once(void *(*dlopen)(const char *,int),void *(*dlsym)(void *,const char *),int(*dlclose)(void *),int option)const; /// @note  jvm should be created only one time per process>
    
    /// @note load shared lib using embedded util. RTLD_LAZY | RTLD_NODELETE as default. for more detail : definition inside <dlfcn.h>
    virtual void create_once(int dl_options = 0|8)const; 
    virtual JNIEnv * env()const;
protected:
    JVMInternal * m = 0;
};

using java_vm  = JVM;


} //namespace jni{
} //namespace kautil{




#endif
