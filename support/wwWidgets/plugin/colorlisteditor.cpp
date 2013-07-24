//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "colorlisteditor.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QColorDialog>
#include <QPushButton>
#include "colormodel.h"
#include <QDir>
#include <QFile>


ColorListEditor::ColorListEditor(QWidget* parent, Qt::WFlags fl)
        : QDialog( parent, fl ) {
    setupUi(this);
    loadColorMaps();
    up->setIcon(QPixmap(":/trolltech/formeditor/images/win/up.png"));
    down->setIcon(QPixmap(":/trolltech/formeditor/images/win/down.png"));
    plus->setIcon(QPixmap(":/trolltech/formeditor/images/win/plus.png"));
    minus->setIcon(QPixmap(":/trolltech/formeditor/images/win/minus.png"));

    clear->setIcon(QPixmap(":/trolltech/formeditor/images/win/editdelete.png"));
    model = new ColorModel(this);
    list->setModel(model);
    m_loadStandard = new QAction(this);
    m_loadStandard->setText(tr("Standard colors"));
    m_loadStandard->setIcon(QPixmap(":/trolltech/formeditor/images/widgets/widgetstack.png"));
    standardColors->setDefaultAction(m_loadStandard);
    /* buttons */
    connect(m_loadStandard, SIGNAL(triggered()), SLOT(setStandardColors()));
    connect(plus, SIGNAL(clicked()), SLOT(addNewRow()));
    connect(minus, SIGNAL(clicked()), SLOT(removeCurrentRow()));
    connect(up, SIGNAL(clicked()), SLOT(moveUp()));
    connect(down, SIGNAL(clicked()), SLOT(moveDown()));
    connect(clear, SIGNAL(clicked()), SLOT(clearModel()));
    connect(chooseColor, SIGNAL(clicked()), SLOT(chooseNewColor()));

    /* data updates */
    connect(list->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), SLOT(updateFields(const QModelIndex&)));
    connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), SLOT(updateFields(const QModelIndex&)));
    connect(name, SIGNAL(textEdited ( const QString & )), SLOT(updateText(const QString &)));


    updateFields(QModelIndex());
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(false);
    okButton->setAutoDefault(false);
    name->ensurePolished();
    QFontMetrics fm = name->fontMetrics();
    name->setMinimumWidth(fm.width("XXXXXXX"));
}

ColorListEditor::~ColorListEditor() {}

/*$SPECIALIZATION$*/

void ColorListEditor::setStandardColors() {
    if (QMessageBox::question(this, tr("Confirm"),
                              tr("Setting standard colors will replace all defined colors with standard SVG colors."
                                 "Are you sure you want to do that?"),
                              QMessageBox::Yes, QMessageBox::No)==QMessageBox::Yes) {
        model->clear();
        foreach(QString col, QColor::colorNames()) {
            QColor c;
            c.setNamedColor(col);
            QStandardItem *item = new QStandardItem;
            item->setText(col);
            item->setData(c, Qt::DecorationRole);
            model->appendRow(item);
        }
        standardColors->setDefaultAction(m_loadStandard);
    }
}

ColorModel * ColorListEditor::colorModel() const {
    return model;
}

void ColorListEditor::addNewRow() {
    QStandardItem *item = new QStandardItem;
    item->setText(tr("New item"));
    model->appendRow(item);
    QModelIndex index = model->index(model->rowCount()-1, 0);
    list->setCurrentIndex(index);
    list->edit(index);
}

void ColorListEditor::removeCurrentRow() {
    QModelIndex curr = list->currentIndex();
    if (curr.isValid()) {
        model->removeRow(curr.row());
    }
}

void ColorListEditor::chooseNewColor() {
    QModelIndex ind = list->currentIndex();
#if QT_VERSION >= 0x040500
    QColor c = QColorDialog::getColor(ind.isValid() ? qvariant_cast<QColor>(ind.data(Qt::DecorationRole)) : Qt::white, this,  tr("Choose color"), QColorDialog::ShowAlphaChannel);
#else
    QColor c = QColorDialog::getColor(ind.isValid() ? qvariant_cast<QColor>(ind.data(Qt::DecorationRole)) : Qt::white, this);
#endif
    if (c.isValid()) {
        QPixmap px(12, 12);
        px.fill(c);
        chooseColor->setIcon(px);

        if(ind.isValid()){
            model->setData(ind, c, Qt::DecorationRole);
            list->edit(QModelIndex());
        }
    }
}

void ColorListEditor::moveUp() {
    QModelIndex ind = list->currentIndex();
    if (!ind.isValid() || ind.row()==0)
        return;
    QModelIndex ind2 = ind.sibling(ind.row()-1, ind.column());
    QMap<int, QVariant> dat = model->itemData(ind);
    QMap<int, QVariant> dat2 = model->itemData(ind2);
    model->setItemData(ind, dat2);
    model->setItemData(ind2, dat);
    list->setCurrentIndex(ind2);
}

void ColorListEditor::moveDown() {
    QModelIndex ind = list->currentIndex();
    if (!ind.isValid() || ind.row()>=model->rowCount()-1)
        return;
    QModelIndex ind2 = ind.sibling(ind.row()+1, ind.column());
    QMap<int, QVariant> dat = model->itemData(ind);
    QMap<int, QVariant> dat2 = model->itemData(ind2);
    model->setItemData(ind, dat2);
    model->setItemData(ind2, dat);
    list->setCurrentIndex(ind2);
}

void ColorListEditor::clearModel() {
    if (QMessageBox::question(this, tr("Confirm"),
                              tr("This operation will delete all defined colors."
                                 "Are you sure you want to do that?"),
                              QMessageBox::Yes, QMessageBox::No)==QMessageBox::Yes) {
        model->clear();
        updateFields(QModelIndex());
    }
}

void ColorListEditor::updateFields(const QModelIndex &index)
{
    bool valid = index.isValid();
    chooseColor->setEnabled(valid);
    name->setEnabled(valid);
    minus->setEnabled(valid);
    up->setEnabled(valid && index.row()>0);
    down->setEnabled(valid && index.row()<model->rowCount()-1);
    if(!valid)
        return;
    QColor col = qvariant_cast<QColor>(index.data(Qt::DecorationRole));
    QString nam = index.data(Qt::DisplayRole).toString();
    if(col.isValid()){
        QPixmap px(12,12);
        px.fill(col);
        chooseColor->setIcon(px);
    } else {
        chooseColor->setIcon(QIcon());
    }
    name->setText(nam);
        QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(false);
    okButton->setAutoDefault(false);
}

void ColorListEditor::updateText(const QString &s)
{
    QModelIndex ind = list->currentIndex();
        if(ind.isValid()){
            model->setData(ind, s, Qt::DisplayRole);
        }
}

void ColorListEditor::setColorModel(ColorModel *m)
{
    disconnect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateFields(const QModelIndex&)));
    delete model;
    model = m;
    list->setModel(model);
    connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), SLOT(updateFields(const QModelIndex&)));
    connect(list->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), SLOT(updateFields(const QModelIndex&)));
    updateFields(QModelIndex());
}

void ColorListEditor::loadColorMaps()
{
    m_colormaps.clear();
    QDir colormaps(":/colormaps");
    QFileInfoList maps = colormaps.entryInfoList(QStringList() << "*.cmap");
    foreach(QFileInfo mapInfo, maps){
        QFile mapFile(mapInfo.filePath());
        if(!mapFile.open(QFile::ReadOnly))
            continue;
        QList<ColEntry> colors;
        QString mapName=mapInfo.baseName();
        QString line;
        QRegExp namrx("Name: (.*)");
        QRegExp colorrx("\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(.*)");

        while(!mapFile.atEnd()){
            line = mapFile.readLine().simplified();
            if(colorrx.exactMatch(line)){

                ColEntry entry;
                entry.color = QColor(colorrx.cap(1).toInt(), colorrx.cap(2).toInt(), colorrx.cap(3).toInt());
                entry.name = colorrx.cap(4);
                colors << entry;
            } else if(namrx.exactMatch(line)){
                mapName = namrx.cap(1);
            }
        }
        mapFile.close();
        if(colors.isEmpty())
            continue;
        m_colormaps[mapName] = colors;
        QAction *act = new QAction(this);
        act->setText(mapName);
        act->setIcon(QPixmap(":/hspicker.png"));
        connect(act, SIGNAL(triggered()), this, SLOT(setColorMap()));
        standardColors->addAction(act);
    }
}

void ColorListEditor::setColorMap()
{
    QAction *snd = qobject_cast<QAction*>(sender());
    Q_ASSERT(snd!=0);
    if(snd==m_loadStandard){
        setStandardColors();
        return;
    }
    if(!m_colormaps.contains(snd->text()))
        return;
    QList<ColEntry> &map = m_colormaps[snd->text()];

    if (QMessageBox::question(this, tr("Confirm"),
                              tr("Setting a color map will replace all already defined colors with colors defined in the map."
                                 "Are you sure you want to do that?"),
                              QMessageBox::Yes, QMessageBox::No)==QMessageBox::Yes) {
        model->clear();
        foreach(ColEntry entry, map) {
            model->addColor(entry.color, entry.name);
        }
        standardColors->setDefaultAction(snd);
    }
}


