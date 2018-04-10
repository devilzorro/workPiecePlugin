#include "workP.h"
#include <module/pluginimpl.h>

XBEGIN_DEFINE_MODULE()
XDEFINE_CLASSMAP_ENTRY(FCworkPiece)
XEND_DEFINE_MODULE_DLL()

OUTAPI bool x3InitializePlugin()
{
    return true;
}

OUTAPI void x3UninitializePlugin()
{
}
