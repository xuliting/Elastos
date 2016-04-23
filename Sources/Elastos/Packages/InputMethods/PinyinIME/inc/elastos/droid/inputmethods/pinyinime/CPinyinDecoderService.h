
#ifndef __ELASTOS_DROID_INPUTMETHODS_PINYINIME_CPINYINDECODERSERVICE_H__
#define __ELASTOS_DROID_INPUTMETHODS_PINYINIME_CPINYINDECODERSERVICE_H__

#include "_CPinyinDecoderService.h"
#include <elastos/droid/app/Service.h>
// #include <sys/types.h>

namespace Elastos {
namespace Droid {
namespace Inputmethods {
namespace PinyinIME {

/**
 * This class is used to separate the input method kernel in an individual
 * service so that both IME and IME-syncer can use it.
 */
CarClass(CPinyinDecoderService)
    , public Service
{
public:
    CPinyinDecoderService();

    CARAPI constructor();

    CARAPI OnCreate();

    CARAPI OnDestroy();

    CARAPI OnBind(
        /* [in] */ IIntent* intent,
        /* [out] */ IBinder** binder);

private:
    // Get file name of the specified dictionary
    CARAPI_(Boolean) GetUsrDictFileName(
        /* [in, out] */ ArrayOf<Byte>* usr_dict);

    CARAPI InitPinyinEngine();

    static CARAPI_(String) Char16ArrayToString(
        /* [in] */ Char16* buf,
        /* [in] */ Int32 size);

private:
    static CARAPI_(Boolean) NativeImOpenDecoder(
        /* [in] */ ArrayOf<Byte>* fn_sys_dict,
        /* [in] */ ArrayOf<Byte>* fn_usr_dict);

    static CARAPI_(Boolean) NativeImOpenDecoderFd(
        /* [in] */ IFileDescriptor* fd,
        /* [in] */ Int64 startOffset,
        /* [in] */ Int64 length,
        /* [in, out] */ ArrayOf<Byte>* fn_usr_dict);

    static CARAPI_(void) NativeImSetMaxLens(
        /* [in] */ Int32 maxSpsLen,
        /* [in] */ Int32 maxHzsLen);

    static CARAPI_(Boolean) NativeImCloseDecoder();

    static CARAPI_(Int32) NativeImSearch(
        /* [in] */ ArrayOf<Byte>* pyBuf,
        /* [in] */ Int32 pyLen);

    static CARAPI_(Int32) NativeImDelSearch(
        /* [in] */ Int32 pos,
        /* [in] */ Boolean is_pos_in_splid,
        /* [in] */ Boolean clear_fixed_this_step);

    static CARAPI_(void) NativeImResetSearch();

    static CARAPI_(Int32) NativeImAddLetter(
        /* [in] */ Byte ch);

    static CARAPI_(String) NativeImGetPyStr(
        /* [in] */ Boolean decoded);

    static CARAPI_(Int32) NativeImGetPyStrLen(
        /* [in] */ Boolean decoded);

    static CARAPI_(AutoPtr<ArrayOf<Int32> >) NativeImGetSplStart();

    static CARAPI_(String) NativeImGetChoice(
        /* [in] */ Int32 choiceId);

    static CARAPI_(Int32) NativeImChoose(
        /* [in] */ Int32 choiceId);

    static CARAPI_(Int32) NativeImCancelLastChoice();

    static CARAPI_(Int32) NativeImGetFixedLen();

    static CARAPI_(Boolean) NativeImCancelInput();

    static CARAPI_(Boolean) NativeImFlushCache();

    static CARAPI_(Int32) NativeImGetPredictsNum(
        /* [in] */ const String& fixedStr);

    static CARAPI_(String) NativeImGetPredictItem(
        /* [in] */ Int32 predictNo);

    // Sync related
    static CARAPI_(String) NativeSyncUserDict(
        /* [in] */ ArrayOf<Byte>* user_dict,
        /* [in] */ const String& tomerge);

    static CARAPI_(Boolean) NativeSyncBegin(
        /* [in] */ ArrayOf<Byte>* user_dict);

    static CARAPI_(Boolean) NativeSyncFinish();

    static CARAPI_(String) NativeSyncGetLemmas();

    static CARAPI_(Int32) NativeSyncPutLemmas(
        /* [in] */ const String& tomerge);

    static CARAPI_(Int32) NativeSyncGetLastCount();

    static CARAPI_(Int32) NativeSyncGetTotalCount();

    static CARAPI_(Boolean) NativeSyncClearLastGot();

    static CARAPI_(Int32) NativeSyncGetCapacity();

private:
    static const Int32 MAX_PATH_FILE_LENGTH;
    static Boolean sInited;

    String mUsr_dict_file;
    AutoPtr<IPinyinDecoderService> mBinder;

    friend class CIPinyinDecoderService;
};

} // namespace PinyinIME
} // namespace Inputmethods
} // namespace Droid
} // namespace Elastos

#endif  //__ELASTOS_DROID_INPUTMETHODS_PINYINIME_CPINYINDECODERSERVICE_H__
