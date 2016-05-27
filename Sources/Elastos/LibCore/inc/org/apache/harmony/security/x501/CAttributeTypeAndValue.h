
#ifndef __ORG_APACHE_HARMONY_SECURITY_X501_CATTRIBUTETYPEANDVALUE_H__
#define __ORG_APACHE_HARMONY_SECURITY_X501_CATTRIBUTETYPEANDVALUE_H__

#include "_Org_Apache_Harmony_Security_X501_CAttributeTypeAndValue.h"
#include <elastos/core/Object.h>

using Elastos::Core::Object;

namespace Org {
namespace Apache {
namespace Harmony {
namespace Security {
namespace X501 {

CarClass(CAttributeTypeAndValue)
    , public Object
    , public IAttributeTypeAndValue
{
public:
    CAR_OBJECT_DECL()

    CAR_INTERFACE_DECL()

    CARAPI AppendName(
        /* [in] */ const String& attrFormat,
        /* [in] */ Elastos::Core::IStringBuilder * pSb);

    CARAPI GetType(
        /* [out] */ Org::Apache::Harmony::Security::Utils::IObjectIdentifierInUtils** ppIdentifier);

    CARAPI constructor(
        /* [in] */ const String& sOid,
        /* [in] */ Org::Apache::Harmony::Security::X501::IAttributeValue * pValue);

private:
    // TODO: Add your private member variables here.
};

}
}
}
}
}

#endif // __ORG_APACHE_HARMONY_SECURITY_X501_CATTRIBUTETYPEANDVALUE_H__
