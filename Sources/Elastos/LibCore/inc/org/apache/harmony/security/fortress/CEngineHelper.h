
#ifndef __ORG_APACHE_HARMONY_SECURITY_FORTRESS_CENGINEHELPER_H__
#define __ORG_APACHE_HARMONY_SECURITY_FORTRESS_CENGINEHELPER_H__

#include "_Org_Apache_Harmony_Security_Fortress_CEngineHelper.h"
#include <elastos/core/Singleton.h>

using Elastos::Core::Singleton;

namespace Org {
namespace Apache {
namespace Harmony {
namespace Security {
namespace Fortress {

CarClass(CEngineHelper)
    , public Singleton
    , public IEngineHelper
{
public:
    CAR_SINGLETON_DECL()

    CAR_INTERFACE_DECL()

    CARAPI SetDoor(
        /* [in] */ ISecurityAccess* door);

    CARAPI GetDoor(
        /* [out] */ ISecurityAccess** door);
};

} // namespace Fortress
} // namespace Security
} // namespace Harmony
} // namespace Apache
} // namespace Org

#endif // __ORG_APACHE_HARMONY_SECURITY_FORTRESS_CENGINEHELPER_H__
