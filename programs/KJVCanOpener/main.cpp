#include <QtGui/QApplication>
#include "KJVCanOpener.h"
#include "CSV.h"

#include <assert.h>

#include <QFile>

#include <QtSql>
#include <QSqlQuery>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QLocale>

#define NUM_BK 66
#define NUM_BK_OT 39
#define NUM_BK_NT 27

const char *g_arrstrBkTblNames[NUM_BK] =
        {	"GEN",
            "EXOD",
            "LEV",
            "NUM",
            "DEUT",
            "JOSH",
            "JUDG",
            "RUTH",
            "SAM1",
            "SAM2",
            "KGS1",
            "KGS2",
            "CHR1",
            "CHR2",
            "EZRA",
            "NEH",
            "ESTH",
            "JOB",
            "PS",
            "PROV",
            "ECCL",
            "SONG",
            "ISA",
            "JER",
            "LAM",
            "EZEK",
            "DAN",
            "HOS",
            "JOEL",
            "AMOS",
            "OBAD",
            "JONAH",
            "MIC",
            "NAH",
            "HAB",
            "ZEPH",
            "HAG",
            "ZECH",
            "MAL",
            "MATT",
            "MARK",
            "LUKE",
            "JOHN",
            "ACTS",
            "ROM",
            "COR1",
            "COR2",
            "GAL",
            "EPH",
            "PHIL",
            "COL",
            "THESS1",
            "THESS2",
            "TIM1",
            "TIM2",
            "TITUS",
            "PHLM",
            "HEB",
            "JAS",
            "PET1",
            "PET2",
            "JOHN1",
            "JOHN2",
            "JOHN3",
            "JUDE",
            "REV"
        };


void buildDbase()
{
    QSqlDatabase myDatabase;
    myDatabase = QSqlDatabase::addDatabase("QSQLITE");
    myDatabase.setDatabaseName("../KJVCanOpener/db/kjvtext.s3db");
//    myDatabase.setDatabaseName("C:/MyData/programs/KJVCanOpener/db/kjvtext.s3db");

//    QMessageBox::information(0,"Database",myDatabase.databaseName());

    if (!myDatabase.open()) {
        QMessageBox::warning(0,"Error","Couldn't open database file.");
        return;
    }

    // Build the BOOK tables:
    for (int i=0; i<NUM_BK; ++i) {
        QSqlQuery queryCreate(myDatabase);
        QString strCmd;

        // Check to see if the table exists already:
        queryCreate.prepare("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name=:table_name");
        queryCreate.bindValue(":table_name", g_arrstrBkTblNames[i]);
        if (!queryCreate.exec()) {
            if (QMessageBox::warning(0, "Database", QString("Table Lookup for \"%1\" Failed!\n%2").arg(g_arrstrBkTblNames[i]).arg(queryCreate.lastError().text()),
                                    QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) break;
        }
        queryCreate.next();
        if (queryCreate.value(0).toInt()) continue;             // If the table exists, skip it

        // Open the table data file:
        QFile fileBook(QString("../KJVCanOpener/db/data/BOOK_%1_%2.csv").arg(i+1, 2, 10, QChar('0')).arg(g_arrstrBkTblNames[i]));
        while (1) {
            if (!fileBook.open(QIODevice::ReadOnly)) {
                if (QMessageBox::warning(0, "Database",
                        QString("Failed to open %1 for reading.").arg(fileBook.fileName()),
                        QMessageBox::Retry, QMessageBox::Cancel) == QMessageBox::Cancel) {
                    myDatabase.close();
                    return;
                }
            } else break;
        }

        // Create the table in the database:
        strCmd = QString("create table %1 "
                        "(ChpVrsNdx INTEGER PRIMARY KEY, NumWrd NUMERIC, bPilcrow NUMERIC, PText TEXT, RText TEXT, Footnote TEXT)").arg(g_arrstrBkTblNames[i]);

        if (!queryCreate.exec(strCmd)) {
            fileBook.close();
            if (QMessageBox::warning(0, "Database",
                    QString("Failed to create table for %1\n%2").arg(g_arrstrBkTblNames[i]).arg(queryCreate.lastError().text()),
                    QMessageBox::Ignore, QMessageBox::Cancel) == QMessageBox::Cancel) {
                myDatabase.close();
                return;
            }
            continue;
        }

        // Read file and populate table:
        CSVstream csv(&fileBook);

        QStringList slHeaders;
        csv >> slHeaders;              // Read Header (and discard)

        while (!csv.atEnd()) {
            QSqlQuery queryInsert(myDatabase);
            QStringList sl;
            csv >> sl;

            assert(sl.count() == 6);
            if (sl.count() < 6) continue;

            strCmd = QString("INSERT INTO %1 "
                        "(ChpVrsNdx, NumWrd, bPilcrow, PText, RText, Footnote) "
                        "VALUES (:ChpVrsNdx, :NumWrd, :bPilcrow, :PText, :RText, :Footnote)").arg(g_arrstrBkTblNames[i]);

            queryInsert.prepare(strCmd);
            queryInsert.bindValue(":ChpVrsNdx", sl.at(0).toInt());
            queryInsert.bindValue(":NumWrd", sl.at(1).toInt());
            queryInsert.bindValue(":bPilcrow", sl.at(2).toInt());
            queryInsert.bindValue(":PText", sl.at(3));
            queryInsert.bindValue(":RText", sl.at(4));
            queryInsert.bindValue(":Footnote", sl.at(5));
            if (!queryInsert.exec()) {
                if (QMessageBox::warning(0, "Database", QString("Insert Failed!\n%1\n  %2  %3").arg(queryInsert.lastError().text()).arg(sl.at(0)).arg(sl.at(3)),
                                        QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) break;
            }
        }

        fileBook.close();
    }

    myDatabase.close();

    QMessageBox::information(0, "Database", "Build Complete!");
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CKJVCanOpener w;
    w.show();

    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

//    if (argc > 1) {
//        if (stricmp(argv[1], "builddb") == 0) {
            buildDbase();
            return 0;
//        }
//    }
//
//    return a.exec();
}
