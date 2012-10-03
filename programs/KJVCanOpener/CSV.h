#ifndef CSV_H
#define CSV_H

#include <QList>
#include <QStringList>
#include <QIODevice>
#include <QBuffer>
#include <QTextStream>

class CSVstream {
public:
    CSVstream();
    CSVstream(QIODevice *iod);
    CSVstream(QString *str, QIODevice::OpenMode filemode);

    QIODevice *device();
    void setDevice(QIODevice * iod);
    void unsetDevice();

    bool atEnd() { return m_stream.atEnd(); }
    bool atEndOfLine() { return atEnd() || m_atEndOfLine; } // for reading
    bool atBeginningOfLine() { return m_atBeginningOfLine; } // for writing
    CSVstream &endLine();


    // utility method to transpose list of columsn to a list of rows
    static QList<QStringList> transpose(const QList<QStringList> &values);


    // template overload allowing this class to handle any data types that QTextStream can
    template <class T> CSVstream &operator <<(const T&val) {
        QBuffer tmp;
        tmp.open(QIODevice::WriteOnly);
        QTextStream stream(&tmp);
        stream.setCodec("UTF-16");
        stream << val;
        tmp.close();
        return (*this << QString((QChar *)tmp.buffer().data(),tmp.buffer().size() / sizeof(QChar)));
    }

    template <class T> CSVstream &operator >>(T&val) {
        QString s;
        CSVstream &r = (*this >> s);
        QByteArray data(s.utf16(),s.size() * sizeof(ushort));
        QBuffer tmp(data);
        tmp.open(QIODevice::ReadOnly);
        QTextStream stream(&tmp);
        stream.setCodec("UTF-16");
        stream >> val;
        tmp.close();
        return r;
    }
    static QString escape(const QString &s,bool forceQuote = false,QChar delimChar = L',');

    CSVstream &operator << (const QString &s); // read individual fields
    CSVstream &operator >> (QString &s);
    CSVstream &operator << (const QStringList &sl); // individual rows
    CSVstream &operator >> (QStringList &sl);

    QList<QStringList> readAll();

    // excel seems to do identical quoting for tab-delimited text,
    // and I've seen the same with '|', so we'll allow changing the delimiter
    QChar delimiter() { return m_delimchar; }
    void setDelimiter(QChar ch) { m_delimchar = ch; }

private:
    QTextStream m_stream;
    bool m_atEndOfLine;
    bool m_atBeginningOfLine;
    QString m_unget;
    QChar m_delimchar;
};

typedef CSVstream& (*CSVFUNC)(CSVstream&);// manipulator function
inline CSVstream& operator<<(CSVstream&s, CSVFUNC f ) {
    return (*f)(s);
}

CSVstream &endl( CSVstream &s );	// insert EOL ('\n')
CSVstream &flush( CSVstream &s );	// flush output

#endif // CSV_H
