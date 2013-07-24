#include <QApplication>
#include <QwwRichTextEdit>

int main(int argc, char **argv){
    QApplication app(argc, argv);
    QwwRichTextEdit textEdit;
    textEdit.setFocus();
    textEdit.setFont(QFont("Serif", 14));
    textEdit.setBold(true);
    textEdit.setItalic(true);
    textEdit.textCursor().insertText("Hello world!");
    textEdit.resize(500, 300);
    textEdit.show();
    return app.exec();
}
