#include "plugin.h"
#include "custom_object_view.h"

#include <qqml.h>

void MyPluginPlugin::registerTypes(const char *uri)
{
  // @uri dkqmlcomponents
  qmlRegisterType<CustomObjectView>(uri, 1, 0, "CustomObjectView");
}

