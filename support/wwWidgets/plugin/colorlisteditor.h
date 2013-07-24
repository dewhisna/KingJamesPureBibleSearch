//
// C++ Interface:
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef COLORLISTEDITOR_H
#define COLORLISTEDITOR_H

#include <QDialog>
#include "ui_colorlisteditor.h"
class ColorModel;
/**
 *  @internal
 *
 */
class ColorListEditor : public QDialog, private Ui::ColorListEditor
{
  Q_OBJECT

public:
  ColorListEditor(QWidget* parent = 0, Qt::WFlags fl = 0 );
  ~ColorListEditor();
  /*$PUBLIC_FUNCTIONS$*/
    ColorModel *colorModel() const;
    void setColorModel(ColorModel*);
public slots:
  /*$PUBLIC_SLOTS$*/

protected:
  /*$PROTECTED_FUNCTIONS$*/
  ColorModel *model;
  QAction *m_loadStandard;
  void loadColorMaps();
  struct ColEntry {
    QColor color;
    QString name;
  };
  QMap<QString, QList<ColEntry> > m_colormaps;
protected slots:
  /*$PROTECTED_SLOTS$*/
    void setColorMap();
    void setStandardColors();
    void clearModel();
    void addNewRow();
    void removeCurrentRow();
    void chooseNewColor();
    void moveUp();
    void moveDown();
    void updateFields(const QModelIndex &);
    void updateText(const QString &);
};

#endif

