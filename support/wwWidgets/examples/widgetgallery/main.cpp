//
// C++ Implementation: main
//
// Description: 
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QApplication>
#include "ui_gallery.h"

int main(int argc, char **argv){
  QApplication app(argc, argv);
  QwwNavigationBar w;
  Ui::Gallery ui;
  ui.setupUi(&w);
  w.show();
  return app.exec();
}
