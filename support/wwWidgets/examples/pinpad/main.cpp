#include <QApplication>
#include <QwwNumPad>
#include <QLineEdit>
#include <QLayout>

class Widget : public QWidget {
    Q_OBJECT
public:
    Widget() : QWidget(){
        QwwNumPad *pad = new QwwNumPad;
        lineEdit = new QLineEdit;
        lineEdit->setReadOnly(true);
        lineEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        connect(pad, SIGNAL(numberClicked(int)), this, SLOT(handleClick(int)));
        connect(pad, SIGNAL(hashClicked()), lineEdit, SLOT(clear()));
        QVBoxLayout *l = new QVBoxLayout(this);
        l->addWidget(lineEdit);
        l->addWidget(pad);
        l->setSizeConstraint(QLayout::SetFixedSize);
    }
private:
    QLineEdit *lineEdit;
private slots:
    void handleClick(int val){
        lineEdit->setText(lineEdit->text()+QString::number(val));
    }
};

#include "main.moc"

int main(int argc, char **argv){
    QApplication app(argc, argv);
    Widget w;
    w.show();
    return app.exec();
}
