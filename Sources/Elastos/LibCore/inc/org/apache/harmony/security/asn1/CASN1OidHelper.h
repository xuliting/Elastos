
#ifndef __ORG_APACHE_HARMONY_SECURITY_ASN1_CASN1OIDHELPER_H__
#define __ORG_APACHE_HARMONY_SECURITY_ASN1_CASN1OIDHELPER_H__

#include "_Org_Apache_Harmony_Security_Asn1_CASN1OidHelper.h"
#include <elastos/core/Singleton.h>

using Elastos::Core::Singleton;

namespace Org {
namespace Apache {
namespace Harmony {
namespace Security {
namespace Asn1 {

CarClass(CASN1OidHelper)
    , public Singleton
    , public IASN1OidHelper
{
public:
    CAR_SINGLETON_DECL()

    CAR_INTERFACE_DECL()

    CARAPI GetInstance(
        /* [out] */ IASN1Type** instance);

    CARAPI GetInstanceForString(
        /* [out] */ IASN1Type** instance);

private:
    // TODO: Add your private member variables here.
};

} // namespace Asn1
} // namespace Security
} // namespace Harmony
} // namespace Apache
} // namespace Org

#endif // __ORG_APACHE_HARMONY_SECURITY_ASN1_CASN1OIDHELPER_H__
