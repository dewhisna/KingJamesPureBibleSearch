#include "CSV.h"
#include <QFile>

CSVstream &flush( CSVstream&s )
{
    if ( s.device() )
        qobject_cast<QFile *>(s.device())->flush();
    return s;
}

CSVstream &endl( CSVstream&s )
{
    return s.endLine();
}

CSVstream::CSVstream()
{
    m_stream.setCodec("UTF-8");
    m_atEndOfLine = false;
    m_atBeginningOfLine = true;
    m_delimchar = L',';
}

CSVstream::CSVstream(QIODevice *iod)
    : m_stream(iod)
{
    m_stream.setCodec("UTF-8");
    m_atEndOfLine = false;
    m_atBeginningOfLine = true;
    m_delimchar = L',';
}

CSVstream::CSVstream(QString *str, QIODevice::OpenMode filemode)
    : m_stream(str,filemode)
{
    m_atEndOfLine = false;
    m_atBeginningOfLine = true;
    m_delimchar = L',';
}


//converts a (potentially ragged) list of rows to a rectangular list of columns
QList<QStringList> CSVstream::transpose(const QList<QStringList> &values)
{
    int maxRows = 0; // number of rows in the output
    for(int row = 0; row < values.count(); ++row)
        maxRows = qMax(maxRows,values[row].count());

    QList<QStringList> tValues;
    for(int row = 0; row < maxRows; ++row)
    {
        tValues.append(QStringList());
        for(int col = 0; col < values.count(); ++col)
        {
            if(row < values[col].count())
                tValues[row].append(values[col][row]);
            else
                tValues[row].append(QString());
        }
    }
    return tValues;
}


QIODevice *CSVstream::device() { return m_stream.device(); }
void CSVstream::setDevice(QIODevice * iod) { m_stream.setDevice(iod); }
void CSVstream::unsetDevice() { m_stream.setDevice(0); }

CSVstream &CSVstream::endLine()
{
    m_stream << endl;
    m_atBeginningOfLine = true;
    return *this;
}

// based on the field quoting rules of http://www.creativyst.com/Doc/Articles/CSV/CSV01.htm
// conforms to RFC4180 (definition of text/csv) except for ignoring unquoted leading whitespace
QString CSVstream::escape(const QString &s,bool forceQuote,QChar delimChar)
{
    bool quoted = false;
    QString outS;
    if(s.at(0) == ' ' || s.at(0) == '\t')
        quoted = true; // leading whitespace has to be quoted
    for(int i = 0; i < s.length(); ++i)
    {
        QChar ch = s.at(i);
        if(ch == '"') {
            outS.append(ch); // insert it an extra time
            quoted = true;
        } else if(ch == delimChar || ch == '\n') {
            quoted = true;
        }
        outS.append(ch);
    }
    if(forceQuote || quoted || (outS.isEmpty() && !s.isNull())) {
        outS.prepend('"');
        outS.append('"');
    }
    return outS;
}

CSVstream &CSVstream::operator << (const QString &s)
{
    bool forceQuote = false;
    // Excel will (rather stupidly) detect a file as SYLK instead of CSV
    // if it begins with the characters ID. Force quoting of the cell
    // (so it starts with "ID instead) if this looks like a potential problem.
    // See http://support.microsoft.com/kb/323626
    if(m_stream.device()->pos() == 0 && s.left(2) == "ID")
        forceQuote = true;
    QString outS = escape(s,forceQuote,m_delimchar);

    if(!m_atBeginningOfLine)
        m_stream << m_delimchar;
    m_atBeginningOfLine = false;

    m_stream << outS;
    return *this;
}

CSVstream &CSVstream::operator >> (QString &s)
{
    m_atEndOfLine = false;
    s = QString();

    bool quoted = false;
    bool quoteLiteral = false;
    bool nonWhitespaceSeen = false;
    bool atEndOfField = false;

    QChar ch;
    while(!atEndOfField) {
        if(m_unget.length()) {
            ch = m_unget.left(1)[0];
            m_unget = m_unget.mid(1);
        } else {
            m_stream >> ch;
        }

        if(!nonWhitespaceSeen) { // quoting can only begin at the first character
            if(ch == ' ' || ch == '\t') {
                continue; // ignore all leading whitespace
            } else {
                nonWhitespaceSeen = true;
            }
            if(ch == '"') {
                s = ""; // not a null field anymore, but still an empty string
                quoted = true;
                continue;
            }
        }

        if(!quoted) {
            // handle linefeed variations - CR or CRLF or LF all get consumed in one step, and returned as '\n'
            if(ch == '\x0D') { //CR
                m_stream >> ch;
                if(ch == '\x0A') // windows-style CRLF
                    ch = '\n';
                else { // something unexpected, guess it was a mac-style bare CR
                    m_unget.prepend(ch);
                    ch = '\n';
                }
            } else if(ch == '\x0A') { // unix-style LF
                ch = '\n';
            }
        }

        if(ch == '"') {
            if(!quoted) {
                // no special treatment in this case
            } else if(quoteLiteral) {
                quoteLiteral = false;
            } else {
                quoteLiteral = true;
                continue;
            }
        } else if(quoteLiteral) { // un-doubled " in quoted string, that's the end of the quoted portion
            quoted = false;
            quoteLiteral = true;
        }

        if( !quoted && ch == m_delimchar) {
            atEndOfField = true;
        } else if(!quoted && ch == '\n') {
            atEndOfField = true;
            m_atEndOfLine = true;
        } else {
            s.append(ch);
        }
    }
    return *this;
}

CSVstream &CSVstream::operator << (const QStringList &sl)
{
    for(QStringList::const_iterator i = sl.begin(); i != sl.end(); ++i)
        *this << *i;
    return (*this << endl);
}

CSVstream &CSVstream::operator >> (QStringList &sl)
{
    sl.clear();
    do {
        QString s;
        *this >> s;
        sl.append(s);
    } while(!atEndOfLine());
    return *this;
}

QList<QStringList> CSVstream::readAll()
{
    QList<QStringList> sll;
    while(!atEnd()) {
        QStringList sl;
        *this >> sl;
        sll.append(sl);
    }
    return sll;
}

