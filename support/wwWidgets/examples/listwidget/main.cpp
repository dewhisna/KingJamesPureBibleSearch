#include <QwwListWidget>
#include <QApplication>
#include <QLayout>
#include <QToolButton>
#include <QInputDialog>
#include <QwwListNavigator>

class Widget : public QWidget {
    Q_OBJECT
public:
    Widget() : QWidget() {
        listWidget = new QwwListWidget;
        listWidget->addItems(QStringList() << "A" << "B" << "C" << "D");

        QToolButton *add = new QToolButton;
        add->setText(tr("Add"));
        QToolButton *rem = new QToolButton;
        rem->setText(tr("Remove"));
        QToolButton *up = new QToolButton;
        up->setText(tr("Up"));
        QToolButton *down = new QToolButton;
        down->setText(tr("Down"));

        QHBoxLayout *hlayout = new QHBoxLayout;
        hlayout->addWidget(add);
        hlayout->addWidget(rem);
        hlayout->addStretch();
        hlayout->addWidget(up);
        hlayout->addWidget(down);

        QHBoxLayout *wl = new QHBoxLayout;
        wl->addWidget(listWidget);
        nav = new QwwListNavigator;
        nav->setOrientation(Qt::Vertical);
        wl->addWidget(nav);
        nav->setListWidget(listWidget);

        QVBoxLayout *l = new QVBoxLayout(this);
        l->addLayout(wl);
        l->addLayout(hlayout);


        connect(add, SIGNAL(clicked()), this, SLOT(addItem()));
        connect(rem, SIGNAL(clicked()), listWidget, SLOT(removeCurrent()));
        connect(up, SIGNAL(clicked()), listWidget, SLOT(moveCurrentUp()));
        connect(down, SIGNAL(clicked()), listWidget, SLOT(moveCurrentDown()));
        connect(listWidget, SIGNAL(currentAvailable(bool)), rem, SLOT(setEnabled( bool )));
        connect(listWidget, SIGNAL(moveUpAvailable(bool)), up, SLOT(setEnabled( bool )));
        connect(listWidget, SIGNAL(moveDownAvailable(bool)), down, SLOT(setEnabled( bool )));
        listWidget->setCurrentRow(0);
    }
private slots:
    void addItem() {
        QString i = QInputDialog::getText(this, tr("Add item"), tr("Enter item text"));
        if (i.isEmpty())
            return;
        listWidget->addItem(i);
        listWidget->setCurrentRow(listWidget->count()-1);
    }
protected:
    QwwListWidget *listWidget;
    QwwListNavigator *nav;
};

#include "main.moc"

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    Widget w;
    w.show();
    return app.exec();
}
