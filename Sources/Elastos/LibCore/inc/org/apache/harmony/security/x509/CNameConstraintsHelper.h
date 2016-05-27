
#ifndef __ORG_APACHE_HARMONY_SECURITY_X509_CNAMECONSTRAINTSHELPER_H__
#define __ORG_APACHE_HARMONY_SECURITY_X509_CNAMECONSTRAINTSHELPER_H__

#include "_Org_Apache_Harmony_Security_X509_CNameConstraintsHelper.h"
#include <elastos/core/Singleton.h>

using Elastos::Core::Singleton;

namespace Org {
namespace Apache {
namespace Harmony {
namespace Security {
namespace X509 {

CarClass(CNameConstraintsHelper)
    , public Singleton
    , public INameConstraintsHelper
{
public:
    CAR_SINGLETON_DECL()

    CAR_INTERFACE_DECL()

    CARAPI Decode(
        /* [in] */ ArrayOf<Byte> * pEncoding,
        /* [out] */ Org::Apache::Harmony::Security::X509::INameConstraints ** ppObject);

    CARAPI GetASN1(
        /* [out] */ Org::Apache::Harmony::Security::Asn1::IASN1Sequence ** ppAsn1);

private:
    // TODO: Add your private member variables here.
};

}
}
}
}
}

#endif // __ORG_APACHE_HARMONY_SECURITY_X509_CNAMECONSTRAINTSHELPER_H__
