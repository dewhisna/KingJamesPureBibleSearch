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

#include "ui_colorpickers.h"
#include <QApplication>

int main(int argc, char **argv){
  QApplication app(argc, argv);
  QWidget w;
  Ui::ColorPickers ui;
  ui.setupUi(&w);
  w.show();
  return app.exec();
}
