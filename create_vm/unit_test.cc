

#ifdef TMAIN_KAUTIL_JNI_CREATE_VM_STATIC


#if defined(WIN32) || defined(WIN64) || defined(_MSC_VER)

//https://stackoverflow.com/questions/29174938/googletest-and-memory-leaks
#include <crtdbg.h>
class MemoryLeakDetector {
public:
    MemoryLeakDetector() { _CrtMemCheckpoint(&memState_); }
    ~MemoryLeakDetector() {
        _CrtMemState stateNow, stateDiff;
        _CrtMemCheckpoint(&stateNow);
        int diffResult = _CrtMemDifference(&stateDiff, &memState_, &stateNow);
        if (diffResult)reportFailure(stateDiff.lSizes[1]); 
    }
private:
    void reportFailure(unsigned int unfreedBytes) { }
    _CrtMemState memState_;
};
auto kMemLeakCheck = MemoryLeakDetector();
#endif

#include "jni.h"
#include "create_vm.h"
int tmain_create_vm(const char * directory_of_system_jars,int dlop){
    // one instance per process
    auto vm=kautil::jni::java_vm();{
        try{
            printf("set up java vm info\n");fflush(stdout);
            vm.Xms("1024m");
            vm.Xmx("2048m");
            vm.add_java_class_path_with_directory(directory_of_system_jars);
            printf("create java vm\n"); fflush(stdout);
            vm.create_once(dlop);
            printf("created java vm successfully\n");fflush(stdout);
        }catch(...){
            printf("error");
        }
    }
    
    auto env = vm.env(); 
    auto jstr = env->NewStringUTF("this is jstring");
    if(env->ExceptionOccurred()) env->ExceptionDescribe();
    
    auto cls = env->FindClass("java/lang/System");
    auto cstr = env->GetStringUTFChars((jstring)jstr,nullptr);
    printf("%s\n",cstr); fflush(stdout);
    env->ReleaseStringUTFChars(jstr,cstr);
    
    
    env->DeleteLocalRef(jstr);
    return 0;
}

int main(){
    auto path_to_jars = "C:/Users/ka/.gradle/caches/modules-2/files-2.1";
    return tmain_create_vm(path_to_jars,/*0|*/8);
}


#endif
