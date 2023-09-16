#include "create_vm.h"

#include "jni.h"
typedef jint (JNICALL *JNI_CreateJavaVM_func)(JavaVM **pvm, void **penv, void *args);

#include "kautil/sharedlib/sharedlib.h"
#include <vector>
#include <string>
#include <filesystem>


// https://docs.oracle.com/cd/E26576_01/doc.312/e24938/list-jvm-options.htm#GSRFM00180
static auto kPrefixXmx="-Xmx";
static auto kPrefixXms="-Xms";
static auto kPrefixXss="-Xss";
static auto kPrefixJavaClassPath="-Djava.class.path=";
constexpr auto kMaxOp = 256;

namespace kautil{
namespace jni{


struct fail_to_open_dll : std::exception{
    fail_to_open_dll(const char path_to_so[]) : sentence(std::string{"fail to open java vm : "}+path_to_so){}
    const char* what() const   noexcept override{ return sentence.c_str(); }
    std::string sentence;
};

struct kautil_jni_create_vm_exception : std::exception{
    JNIEnv * env=0;
    kautil_jni_create_vm_exception(JNIEnv * env) : env(env){}
    const char* what() const   noexcept override{ 
        static auto kRet = " from kautil_jni_create_vm_exception";
        env->ExceptionOccurred();
        return kRet; 
    }
};


struct JVMInternal{
    int(*dlclose)(void *)=0;
    void *dl=0;
    
//    JavaVMInitArgs vmarg;
    JavaVMOption op_array[kMaxOp];
    int op_pos=0;
    std::string Xss="";
    std::string Xms="";
    std::string Xmx="";
    std::string java_class_path="";
    std::string path_to_jvm=JVM_SO;
    jint jni_version = JVM_JNI_VERSION;
    JNIEnv * env_=0;
    JavaVM * vm_=0;
    bool initialized = false;
    void create_once(void *(*dlopen)(const char *,int),void *(*dlsym)(void *,const char *),int(*dlclose)(void *),int option);
    void add_array(const char* target_prefix,std::string & target_data){
        if(!target_data.empty()){
            target_data=target_prefix+target_data;
            op_array[op_pos++]=JavaVMOption{(target_data).data(), nullptr};
        }
    }
    ~JVMInternal(){
        if(vm_) vm_->DestroyJavaVM();
        if(dl)dlclose(dl);
    }
};

void JVMInternal::create_once(void *(*dlopen)(const char *,int),void *(*dlsym)(void * ,const char *),int(*dlclose)(void *),int dl_option){
    if(initialized) return;
    initialized = true;
    this->dlclose = dlclose;
    add_array(kPrefixXms,Xms);
    add_array(kPrefixXmx,Xmx);
    add_array(kPrefixXss,Xss);
    add_array(kPrefixJavaClassPath,java_class_path);
    
    
    JavaVMInitArgs vmarg;{
        vmarg.version = jni_version;
        vmarg.options = op_array;
        vmarg.nOptions = op_pos;
    }
    dl = dlopen(path_to_jvm.data(),dl_option);
    if(dl == nullptr)  throw fail_to_open_dll{path_to_jvm.data()};
    
    auto fp_create_jvm = (JNI_CreateJavaVM_func) dlsym(dl,"JNI_CreateJavaVM"); 
    
    vm_ = nullptr;
    env_ = nullptr;
    auto res = fp_create_jvm(&vm_, (void**)&env_, &vmarg);
    if(env_->ExceptionOccurred()){
        env_->ExceptionDescribe();
        throw kautil_jni_create_vm_exception{env_};
    }
    
    
}



//const char* JVM::jdk_version()const{ return m->jdk_version.data(); }
long JVM::jni_version()const{ return m->jni_version; }


void JVM::create_once(void *(*dlopen)(const char *,int),void *(*dlsym)(void * ,const char *),int(*dlclose)(void *),int op) const{ 
    m->create_once(dlopen,dlsym,dlclose,op); 
}


void JVM::create_once(int option)const{
    m->create_once(kautil_dlopen,kautil_dlsym,kautil_dlclose,option); 
} 

JNIEnv * JVM::env() const{ return m->env_; }


JVM::JVM() : m(new JVMInternal){}
JVM::~JVM(){ delete m; }


void JVM::Xmx(const char *value)const { m->Xmx = value; }
void JVM::Xms(const char *value)const { m->Xms=value; }
void JVM::Xss(const char *value)const { m->Xss=value; }
void JVM::add_java_class_path(const char **array, uint64_t length)const {
    for(auto i = 0; i < length; ++i){
        m->java_class_path+=array[i];
        if(i<length-1)m->java_class_path.append(";");
    }
}

void JVM::add_java_class_path_with_directory(const char *directory)const {
    namespace fs=  std::filesystem;
    bool occure = false;
    for(auto & p : fs::recursive_directory_iterator{directory}){
        if(p.path().extension().string()==".jar"){
            occure = true;
            m->java_class_path.append(p.path().string()).append(";");
        }
    }
//    if(occure) m->java_class_path.resize(m->java_class_path.size()-1);
}

void JVM::add_java_class_path(const char *p)const {
    m->java_class_path.empty() ? 
        m->java_class_path.append(";").append(p):
        m->java_class_path.append(p);
}

} //namespace jni{
} //namespace kautil{



extern "C" void* extern_initialize(){ return new kautil::jni::JVM{}; }
extern "C" void extern_finalize(void * instance){  delete reinterpret_cast<kautil::jni::JVM*>(instance); }


