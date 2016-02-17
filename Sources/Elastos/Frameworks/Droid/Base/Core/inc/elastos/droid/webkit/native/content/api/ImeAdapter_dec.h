//This file is autogenerated for
//    ImeAdapter.java
//put this file at the end of the include list
//so the type definition used in this file will be found
#ifndef ELASTOS_IMEADAPTER_CALLBACK_DEC_HH
#define ELASTOS_IMEADAPTER_CALLBACK_DEC_HH


#ifdef __cplusplus
extern "C"
{
#endif
    extern void Elastos_ImeAdapter_nativeAppendUnderlineSpan(Int64 underlinePtr,Int32 start,Int32 end);
    extern void Elastos_ImeAdapter_nativeAppendBackgroundColorSpan(Int64 underlinePtr,Int32 start,Int32 end,Int32 backgroundColor);
    extern Boolean Elastos_ImeAdapter_nativeSendSyntheticKeyEvent(IInterface* caller,Handle32 nativeImeAdapterAndroid,Int32 eventType,Int64 timestampMs,Int32 keyCode,Int32 unicodeChar);
    extern Boolean Elastos_ImeAdapter_nativeSendKeyEvent(IInterface* caller,Handle32 nativeImeAdapterAndroid,IInterface* event,Int32 action,Int32 modifiers,Int64 timestampMs,Int32 keyCode,Boolean isSystemKey,Int32 unicodeChar);
    extern void Elastos_ImeAdapter_nativeSetComposingText(IInterface* caller,Handle32 nativeImeAdapterAndroid,IInterface* text,const String& textStr,Int32 newCursorPosition);
    extern void Elastos_ImeAdapter_nativeCommitText(IInterface* caller,Handle32 nativeImeAdapterAndroid,const String& textStr);
    extern void Elastos_ImeAdapter_nativeFinishComposingText(IInterface* caller,Handle32 nativeImeAdapterAndroid);
    extern void Elastos_ImeAdapter_nativeAttachImeAdapter(IInterface* caller,Handle32 nativeImeAdapterAndroid);
    extern void Elastos_ImeAdapter_nativeSetEditableSelectionOffsets(IInterface* caller,Handle32 nativeImeAdapterAndroid,Int32 start,Int32 end);
    extern void Elastos_ImeAdapter_nativeSetComposingRegion(IInterface* caller,Handle32 nativeImeAdapterAndroid,Int32 start,Int32 end);
    extern void Elastos_ImeAdapter_nativeDeleteSurroundingText(IInterface* caller,Handle32 nativeImeAdapterAndroid,Int32 before,Int32 after);
    extern void Elastos_ImeAdapter_nativeUnselect(IInterface* caller,Handle32 nativeImeAdapterAndroid);
    extern void Elastos_ImeAdapter_nativeSelectAll(IInterface* caller,Handle32 nativeImeAdapterAndroid);
    extern void Elastos_ImeAdapter_nativeCut(IInterface* caller,Handle32 nativeImeAdapterAndroid);
    extern void Elastos_ImeAdapter_nativeCopy(IInterface* caller,Handle32 nativeImeAdapterAndroid);
    extern void Elastos_ImeAdapter_nativePaste(IInterface* caller,Handle32 nativeImeAdapterAndroid);
    extern void Elastos_ImeAdapter_nativeResetImeAdapter(IInterface* caller,Handle32 nativeImeAdapterAndroid);
    extern void Elastos_ImeAdapter_InitCallback(Handle32 cb);
#ifdef __cplusplus
}
#endif


namespace Elastos {
namespace Droid {
namespace Webkit {
namespace Content {
namespace Browser {
namespace Input {

struct ElaImeAdapterCallback
{
    void (*elastos_ImeAdapter_initializeWebInputEvents)(Int32 eventTypeRawKeyDown, Int32 eventTypeKeyUp, Int32 eventTypeChar, Int32 modifierShift, Int32 modifierAlt, Int32 modifierCtrl, Int32 modifierCapsLockOn, Int32 modifierNumLockOn);
    void (*elastos_ImeAdapter_initializeTextInputTypes)(Int32 textInputTypeNone, Int32 textInputTypeText, Int32 textInputTypeTextArea, Int32 textInputTypePassword, Int32 textInputTypeSearch, Int32 textInputTypeUrl, Int32 textInputTypeEmail, Int32 textInputTypeTel, Int32 textInputTypeNumber, Int32 textInputTypeContentEditable);
    void (*elastos_ImeAdapter_focusedNodeChanged)(IInterface* obj, Boolean isEditable);
    void (*elastos_ImeAdapter_populateUnderlinesFromSpans)(IInterface* obj, IInterface* text, Int64 underlines);
    void (*elastos_ImeAdapter_cancelComposition)(IInterface* obj);
    void (*elastos_ImeAdapter_detach)(IInterface* obj);
};

void* ImeAdapter::ElaImeAdapterCallback_Init()
{
    static ElaImeAdapterCallback sElaImeAdapterCallback;

    sElaImeAdapterCallback.elastos_ImeAdapter_initializeWebInputEvents = &ImeAdapter::InitializeWebInputEvents;
    sElaImeAdapterCallback.elastos_ImeAdapter_initializeTextInputTypes = &ImeAdapter::InitializeTextInputTypes;
    sElaImeAdapterCallback.elastos_ImeAdapter_focusedNodeChanged = &ImeAdapter::FocusedNodeChanged;
    sElaImeAdapterCallback.elastos_ImeAdapter_populateUnderlinesFromSpans = &ImeAdapter::PopulateUnderlinesFromSpans;
    sElaImeAdapterCallback.elastos_ImeAdapter_cancelComposition = &ImeAdapter::CancelComposition;
    sElaImeAdapterCallback.elastos_ImeAdapter_detach = &ImeAdapter::Detach;

    Elastos_ImeAdapter_InitCallback((Handle32)&sElaImeAdapterCallback);
    return &sElaImeAdapterCallback;
}

static void* sPElaImeAdapterCallback = ImeAdapter::ElaImeAdapterCallback_Init();

} // namespace Input
} // namespace Browser
} // namespace Content
} // namespace Webkit
} // namespace Droid
} // namespace Elastos

#endif //ELASTOS_IMEADAPTER_CALLBACK_DEC_HH