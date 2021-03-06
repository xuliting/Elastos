//This file is autogenerated for
//    JavaHandlerThread.java
//put this file at the end of the include list
//so the type definition used in this file will be found
#ifndef ELASTOS_JAVAHANDLERTHREAD_CALLBACK_DEC_HH
#define ELASTOS_JAVAHANDLERTHREAD_CALLBACK_DEC_HH


#ifdef __cplusplus
extern "C"
{
#endif
    extern void Elastos_JavaHandlerThread_nativeInitializeThread(IInterface* caller,Handle32 nativeJavaHandlerThread,Int64 nativeEvent);
#ifdef __cplusplus
}
#endif


struct ElaJavaHandlerThreadCallback
{
    AutoPtr<IInterface> (*elastos_JavaHandlerThread_create)(const Elastos::String& name);
    void (*elastos_JavaHandlerThread_start)(IInterface* obj, Int64 nativeThread, Int64 nativeEvent);
};


#endif //ELASTOS_JAVAHANDLERTHREAD_CALLBACK_DEC_HH
