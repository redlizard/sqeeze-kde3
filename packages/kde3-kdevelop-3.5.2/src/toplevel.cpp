#include <kxmlguiclient.h>
#include <kapplication.h>
#include <kconfig.h>


#include "toplevel.h"
#include "core.h"

#include "simplemainwindow.h"

KDevMainWindow *TopLevel::s_instance = 0;

bool TopLevel::mainWindowValid()
{
  return s_instance != 0;
}

KDevMainWindow *TopLevel::getInstance()
{
  if (!s_instance)
  {
    SimpleMainWindow *mainWindow = new SimpleMainWindow(0, "SimpleMainWindow");
    s_instance = mainWindow;
    mainWindow->init();
    kapp->setMainWidget(mainWindow);
  }

  return s_instance;
}

void TopLevel::invalidateInstance(KDevMainWindow *instance)
{
  if ( s_instance == instance )
    s_instance = 0;
}
