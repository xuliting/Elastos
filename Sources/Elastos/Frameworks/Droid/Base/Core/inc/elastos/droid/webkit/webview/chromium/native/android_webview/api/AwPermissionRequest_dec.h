//This file is autogenerated for
//    AwPermissionRequest.java
//put this file at the end of the include list
//so the type definition used in this file will be found
#ifndef ELASTOS_AWPERMISSIONREQUEST_CALLBACK_DEC_HH
#define ELASTOS_AWPERMISSIONREQUEST_CALLBACK_DEC_HH


#ifdef __cplusplus
extern "C"
{
#endif
    extern void Elastos_AwPermissionRequest_nativeOnAccept(IInterface* caller,Handle64 nativeAwPermissionRequest,Boolean allowed);
    extern void Elastos_AwPermissionRequest_InitCallback(Handle64 cb);
#ifdef __cplusplus
}
#endif


namespace Elastos {
namespace Droid {
namespace Webkit {
namespace Webview {
namespace Chromium {
namespace AndroidWebview {
namespace Permission{

struct ElaAwPermissionRequestCallback
{
    AutoPtr<IInterface> (*elastos_AwPermissionRequest_create)(Handle64 nativeAwPermissionRequest, const String& url, Int64 resources);
    void (*elastos_AwPermissionRequest_detachNativeInstance)(IInterface* obj);
};

void* AwPermissionRequest::ElaAwPermissionRequestCallback_Init()
{
    static ElaAwPermissionRequestCallback sElaAwPermissionRequestCallback;

    sElaAwPermissionRequestCallback.elastos_AwPermissionRequest_create = &AwPermissionRequest::Create;
    sElaAwPermissionRequestCallback.elastos_AwPermissionRequest_detachNativeInstance = &AwPermissionRequest::DetachNativeInstance;

    Elastos_AwPermissionRequest_InitCallback((Handle64)&sElaAwPermissionRequestCallback);
    return &sElaAwPermissionRequestCallback;
}

static void* sPElaAwPermissionRequestCallback = AwPermissionRequest::ElaAwPermissionRequestCallback_Init();

} // namespace Permission
} // namespace AndroidWebview
} // namespace Chromium
} // namespace Webview
} // namespace Webkit
} // namespace Droid
} // namespace Elastos

#endif //ELASTOS_AWPERMISSIONREQUEST_CALLBACK_DEC_HH