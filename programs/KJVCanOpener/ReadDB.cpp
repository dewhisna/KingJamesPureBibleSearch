// ReadDB.cpp -- Code to read out database into our working variables
//

#include "dbstruct.h"
#include "CSV.h"

#include <assert.h>

#include <QFile>

#include <QtSql>
#include <QSqlQuery>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QByteArray>

#include <QString>

// Used for debugging:
#ifdef NEVER
#include <QFile>
#include <QTextStream>
#endif

static bool ReadTestamentTable(QSqlDatabase &myDatabase)
{
    // Read the Testament Table

    QSqlQuery query(myDatabase);

    // Check to see if the table exists:
    if (!query.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='TESTAMENT'")) {
        QMessageBox::warning(0, "Database", QString("Table Lookup for \"TESTAMENT\" Failed!\n%1").arg(query.lastError().text()));
        return false;
    }
    query.next();
    if (!query.value(0).toInt()) {
        QMessageBox::warning(0, "Database", "Unable to find \"TESTAMENT\" Table in database!");
        return false;
    }

    g_lstTestaments.clear();
    query.setForwardOnly(true);
    query.exec("SELECT * FROM TESTAMENT");
    while (query.next()) {
        unsigned int nTstNdx = query.value(0).toUInt();
        if (nTstNdx > g_lstTestaments.size()) g_lstTestaments.resize(nTstNdx);
        CTestamentEntry &entryTestament = g_lstTestaments[nTstNdx-1];
        entryTestament.m_strTstName = query.value(1).toString().toStdString();
    }

    return true;
}

static bool ReadTOCTable(QSqlDatabase &myDatabase)
{
    // Read the TOC Table

    QSqlQuery query(myDatabase);

    // Check to see if the table exists:
    if (!query.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='TOC'")) {
        QMessageBox::warning(0, "Database", QString("Table Lookup for \"TOC\" Failed!\n%1").arg(query.lastError().text()));
        return false;
    }
    query.next();
    if (!query.value(0).toInt()) {
        QMessageBox::warning(0, "Database", "Unable to find \"TOC\" Table in database!");
        return false;
    }

    g_lstTOC.clear();
    query.setForwardOnly(true);
    query.exec("SELECT * FROM TOC");
    while (query.next()) {
        unsigned int nBkNdx = query.value(0).toUInt();
        if (nBkNdx > g_lstTOC.size()) g_lstTOC.resize(nBkNdx);
        CTOCEntry &entryTOC = g_lstTOC[nBkNdx-1];
        entryTOC.m_nTstBkNdx = query.value(1).toInt();
        entryTOC.m_nTstNdx = query.value(2).toInt();
        entryTOC.m_strBkName = query.value(3).toString().toStdString();
        entryTOC.m_strBkAbbr = query.value(4).toString().toStdString();
        entryTOC.m_strTblName = query.value(5).toString().toStdString();
        entryTOC.m_nNumChp = query.value(6).toInt();
        entryTOC.m_nNumVrs = query.value(7).toInt();
        entryTOC.m_nNumWrd = query.value(8).toInt();
        entryTOC.m_strCat = query.value(9).toString().toStdString();
        entryTOC.m_strDesc = query.value(10).toString().toStdString();
    }

// Used for debugging:
#ifdef NEVER
    QFile fileTest("testit.txt");
    if (fileTest.open(QIODevice::WriteOnly)) {
        QTextStream ts(&fileTest);
        for (TTOCList::const_iterator itr = g_lstTOC.begin(); itr != g_lstTOC.end(); ++itr) {
            QString strTemp = QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10\r\n").arg(itr->m_nTstBkNdx).arg(itr->m_nTstNdx).arg(QString::fromStdString(itr->m_strBkName)).arg(QString::fromStdString(itr->m_strBkAbbr))
                                    .arg(QString::fromStdString(itr->m_strTblName)).arg(itr->m_nNumChp).arg(itr->m_nNumVrs).arg(itr->m_nNumWrd).arg(QString::fromStdString(itr->m_strCat)).arg(QString::fromStdString(itr->m_strDesc));
            ts << strTemp;
        }
        fileTest.close();
    }
#endif

    return true;
}

static bool ReadLAYOUTTable(QSqlDatabase &myDatabase)
{
    // Read the LAYOUT table:

    QSqlQuery query(myDatabase);

    // Check to see if the table exists:
    if (!query.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='LAYOUT'")) {
        QMessageBox::warning(0, "Database", QString("Table Lookup for \"LAYOUT\" Failed!\n%1").arg(query.lastError().text()));
        return false;
    }
    query.next();
    if (!query.value(0).toInt()) {
        QMessageBox::warning(0, "Database", "Unable to find \"LAYOUT\" Table in database!");
        return false;
    }

    g_mapLayout.clear();

    query.setForwardOnly(true);
    query.exec("SELECT * FROM LAYOUT");
    while (query.next()) {
        uint32_t nBkChpNdx = query.value(0).toUInt();
        CLayoutEntry &entryLayout = g_mapLayout[nBkChpNdx];
        entryLayout.m_nNumVrs = query.value(1).toInt();
        entryLayout.m_nNumWrd = query.value(2).toInt();
    }

// Used for debugging:
#ifdef NEVER
    QFile fileTest("testit.txt");
    if (fileTest.open(QIODevice::WriteOnly)) {
        QTextStream ts(&fileTest);
        for (TLayoutMap::const_iterator itr = g_mapLayout.begin(); itr != g_mapLayout.end(); ++itr) {
            QString strTemp = QString("%1,%2,%3\r\n").arg(itr->first).arg(itr->second.m_nNumVrs).arg(itr->second.m_nNumWrd);
            ts << strTemp;
        }
        fileTest.close();
    }
#endif

    return true;
}

static bool ReadBookTables(QSqlDatabase &myDatabase)
{
    // Read the BOOK tables:

    g_lstBooks.clear();
    g_lstBooks.resize(g_lstTOC.size());
    for (unsigned int i=0; i<g_lstTOC.size(); ++i) {
        QSqlQuery query(myDatabase);

        // Check to see if the table exists:
        if (!query.exec(QString("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='%1'").arg(QString::fromStdString(g_lstTOC[i].m_strTblName)))) {
            QMessageBox::warning(0, "Database", QString("Table Lookup for \"%1\" Failed!\n%2").arg(QString::fromStdString(g_lstTOC[i].m_strTblName)).arg(query.lastError().text()));
            return false;
        }
        query.next();
        if (!query.value(0).toInt()) {
            QMessageBox::warning(0, "Database", QString("Unable to find \"%1\" Table in database!").arg(QString::fromStdString(g_lstTOC[i].m_strTblName)));
            return false;
        }

        TBookEntryMap &mapBook = g_lstBooks[i];

        mapBook.clear();

        query.setForwardOnly(true);
        query.exec(QString("SELECT * FROM %1").arg(QString::fromStdString(g_lstTOC[i].m_strTblName)));
        while (query.next()) {
            uint32_t nChpVrsNdx = query.value(0).toUInt();
            CBookEntry &entryBook = mapBook[nChpVrsNdx];
            entryBook.m_nNumWrd = query.value(1).toInt();
            entryBook.m_bPilcrow = ((query.value(2).toInt() != 0) ? true : false);
            entryBook.SetRichText(query.value(4).toString().toStdString());
            entryBook.m_strFootnote = query.value(5).toString().toStdString();
        }

// Used for debugging:
#ifdef NEVER
        QFile fileTest(QString("testit%1.txt").arg(i, 2, 10, QChar('0')));
        if (fileTest.open(QIODevice::WriteOnly)) {
            CSVstream csv(&fileTest);
            for (TBookEntryMap::const_iterator itr = mapBook.begin(); itr != mapBook.end(); ++itr) {
                QStringList sl;
                sl.push_back(QString("%1").arg(itr->first));
                sl.push_back(QString("%1").arg(itr->second.m_nNumWrd));
                sl.push_back(QString("%1").arg(itr->second.m_bPilcrow));
                sl.push_back(QString::fromStdString(itr->second.GetRichText()));
                sl.push_back(QString::fromStdString(itr->second.m_strFootnote));
                csv << sl;
            }
            fileTest.close();
        }
#endif

    }

    return true;
}

static bool IndexBlobToIndexList(const QByteArray &baBlob, TIndexList &anIndexList)
{
    if ((baBlob.size() % sizeof(uint32_t)) != 0) return false;

    anIndexList.clear();
    for (unsigned int i=0; i<(baBlob.size()/sizeof(uint32_t)); ++i) {
        uint32_t nValue = 0;
        for (unsigned int j=0; j<sizeof(uint32_t); ++j) {
            nValue = nValue << 8;
            nValue = nValue | (baBlob[(i*sizeof(uint32_t))+j] & 0xFF);
        }
        anIndexList.push_back(nValue);
    }

    return true;
}

static bool ReadWORDSTable(QSqlDatabase &myDatabase)
{
    // Read the WORDS table:

    QSqlQuery query(myDatabase);

    // Check to see if the table exists:
    if (!query.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='WORDS'")) {
        QMessageBox::warning(0, "Database", QString("Table Lookup for \"WORDS\" Failed!\n%1").arg(query.lastError().text()));
        return false;
    }
    query.next();
    if (!query.value(0).toInt()) {
        QMessageBox::warning(0, "Database", "Unable to find \"WORDS\" Table in database!");
        return false;
    }

    g_mapWordList.clear();

    query.setForwardOnly(true);
    query.exec("SELECT * FROM WORDS");
    while (query.next()) {
        QString strWord = query.value(1).toString();
        QString strKey = strWord;
        if (query.value(2).toInt() == 0) strKey = strKey.toLower();
        CWordEntry &entryWord = g_mapWordList[strKey.toStdString()];
        entryWord.m_strWord = strWord.toStdString();
        QString strAltWords = query.value(5).toString() + '\n';
        CSVstream csvWord(&strAltWords, QIODevice::ReadOnly);
        while (!csvWord.atEnd()) {
            QString strTemp;
            csvWord >> strTemp;
            if (!strTemp.isEmpty()) entryWord.m_lstAltWords.push_back(strTemp.toStdString());
        }
        if ((!IndexBlobToIndexList(query.value(6).toByteArray(), entryWord.m_ndxlstOT)) ||
            (!IndexBlobToIndexList(query.value(7).toByteArray(), entryWord.m_ndxlstNT)) ||
            (!IndexBlobToIndexList(query.value(8).toByteArray(), entryWord.m_ndxNormalized))) {
            QMessageBox::warning(0, "Database", QString("Bad word indexes for \"%1\"").arg(strWord));
            return false;
        }
        if ((entryWord.m_ndxlstOT.size() != query.value(3).toUInt()) ||
            (entryWord.m_ndxlstNT.size() != query.value(4).toUInt()) ||
            (entryWord.m_ndxNormalized.size() != (query.value(3).toUInt()+query.value(4).toUInt()))) {
            QMessageBox::warning(0, "Database", "Index/Count consistency error in WORDS table!");
            return false;
        }
    }

// Used for debugging:
#ifdef NEVER
    QFile fileTest("testit.txt");
    if (fileTest.open(QIODevice::WriteOnly)) {
        QTextStream ts(&fileTest);
        int cnt = 0;
        for (TWordListMap::const_iterator itr = g_mapWordList.begin(); itr != g_mapWordList.end(); ++itr) {
            cnt++;
            ts << QString("%1,").arg(cnt);
//            ts << "\"" + QString::fromStdString(itr->first) + "\",";
            ts << "\"" + QString::fromStdString(itr->second.m_strWord) + "\",";
            if (strcmp(itr->first.c_str(), itr->second.m_strWord.c_str()) == 0) {
                ts << "1,";
            } else {
                ts << "0,";
            }
            ts << QString("%1,").arg(itr->second.m_ndxlstOT.size());
            ts << QString("%1,").arg(itr->second.m_ndxlstNT.size());
            ts << "\"";
            for (unsigned int i=0; i<itr->second.m_lstAltWords.size(); ++i) {
                if (i!=0) ts << ",";
                ts << QString::fromStdString(itr->second.m_lstAltWords.at(i));
            }
            ts << "\",\"";
            for (unsigned int i=0; i<itr->second.m_ndxlstOT.size(); ++i) {
                if (i!=0) ts << ",";
                ts << QString("%1").arg(itr->second.m_ndxlstOT.at(i));
            }
            ts << "\",\"";
            for (unsigned int i=0; i<itr->second.m_ndxlstNT.size(); ++i) {
                if (i!=0) ts << ",";
                ts << QString("%1").arg(itr->second.m_ndxlstNT.at(i));
            }
            ts << "\",\"";
            for (unsigned int i=0; i<itr->second.m_ndxNormalized.size(); ++i) {
                if (i!=0) ts << ",";
                ts << QString("%1").arg(itr->second.m_ndxNormalized.at(i));
            }
            ts << "\"\n";
        }
        fileTest.close();
    }
#endif

    return true;
}

bool ValidateData()
{
//TTOCList g_lstTOC;
//TLayoutMap g_mapLayout;
//TBookList g_lstBooks;
//TWordListMap g_mapWordList;


    return true;
}

bool ReadDatabase(const char *pstrDatabaseFilename)
{
    QSqlDatabase myDatabase;
    myDatabase = QSqlDatabase::addDatabase("QSQLITE");
    myDatabase.setDatabaseName(pstrDatabaseFilename);

//    QMessageBox::information(0,"Database",myDatabase.databaseName());

    if (!myDatabase.open()) {
        QMessageBox::warning(0,"Error","Couldn't open database file.");
        return false;
    }

    bool bSuccess = true;

    if ((!ReadTestamentTable(myDatabase)) ||
        (!ReadTOCTable(myDatabase)) ||
        (!ReadLAYOUTTable(myDatabase)) ||
        (!ReadBookTables(myDatabase)) ||
        (!ReadWORDSTable(myDatabase)) ||
        (!ValidateData())) bSuccess = false;

    myDatabase.close();

    return bSuccess;
}

