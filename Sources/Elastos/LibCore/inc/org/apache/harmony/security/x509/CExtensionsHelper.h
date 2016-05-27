
#ifndef __ORG_APACHE_HARMONY_SECURITY_X509_CEXTENSIONSHELPER_H__
#define __ORG_APACHE_HARMONY_SECURITY_X509_CEXTENSIONSHELPER_H__

#include "_Org_Apache_Harmony_Security_X509_CExtensionsHelper.h"
#include <elastos/core/Singleton.h>

using Elastos::Core::Singleton;

namespace Org {
namespace Apache {
namespace Harmony {
namespace Security {
namespace X509 {

CarClass(CExtensionsHelper)
    , public Singleton
    , public IExtensionsHelper
{
public:
    CAR_SINGLETON_DECL()

    CAR_INTERFACE_DECL()

    CARAPI GetASN1(
        /* [out] */ Org::Apache::Harmony::Security::Asn1::IASN1Type ** ppAsn1);

private:
    // TODO: Add your private member variables here.
};

}
}
}
}
}

#endif // __ORG_APACHE_HARMONY_SECURITY_X509_CEXTENSIONSHELPER_H__
