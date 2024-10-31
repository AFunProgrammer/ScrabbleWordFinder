#include "scrabblewordfinder.h"
#include "ui_scrabblewordfinder.h"
#include <QResource>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardItemModel>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QException>

// Check if the file path exists
bool checkPathExists(const QString &path) {
    QFileInfo fileInfo(path);
    return fileInfo.exists();
}

// Resolve relative path to full path
QString resolveToFullPath(const QString &relativePath) {
    QDir dir(QDir::current());
    QString fullPath = dir.absoluteFilePath(relativePath);
    return fullPath;
}

int countOccurrences(const QString& str, QChar ch) {
    int count = 0;
    for (int i = 0; i < str.length(); ++i) {
        if (str[i] == ch) {
            count++;
        }
    }
    return count;
}

QSet<QChar> getUniqueLetters(const QString& str) {
    QSet<QChar> uniqueLetters;
    for (QChar ch : str) {
        uniqueLetters.insert(ch);
    }
    return uniqueLetters;
}

class CWordFind {
public:
    CWordFind(){}

    bool OpenDatabase(QString FileName){
        QString filePath = resolveToFullPath(FileName);

        if (!checkPathExists(filePath)) {
            qDebug() << "CWordFile path does not exist.";
            return false; // Exit the constructor if the file doesn't exist
        }

        _db = QSqlDatabase::addDatabase("QSQLITE");
        _db.setDatabaseName(filePath);

        try {
            if (!_db.open()) {
                qDebug() << "Error: Failed to open database." << _db.lastError().text();
                return false;
            }
        } catch (const QException &e) {
            qDebug() << "An exception occurred.";
            return false;
            // Handle specific Qt exceptions if needed
        } catch (const std::exception &e) {
            qDebug() << "Standard exception: " << e.what();
            return false;
        } catch (...) {
            qDebug() << "An unknown exception occurred.";
            return false;
        }

        return true;
    }

    QSqlQuery FindWords(QString Letters,
                        int Length = 99,
                        QString Start = QString(""),
                        QString Contains = QString(""),
                        QString End = QString("")) {
        int nStartLen = (Length < 99 ? Length : 0);
        int nEndLen = (Length < 99 ? Length : Letters.length());

        QString strLikeBuilder = "";

        QString strSelectClause = "SELECT word FROM words ";
        QString strWhereClause = QString("WHERE length BETWEEN %0 AND %1 ").arg(nStartLen).arg(nEndLen);
        QString strAndClause = "AND (";
        QString strOrClause = "AND (";
        QString strLikeClause = QString(" AND word LIKE '%0%%%1%%%2'").arg(Start).arg(Contains).arg(End);
        QString strOrderBy = "ORDER BY length DESC,word ASC;";


        for(int nLetter = 0; nLetter < 26; nLetter++){
            int nCount = countOccurrences(Letters,QChar((char)('a'+nLetter)));
            if (nCount > 0 )
                strAndClause += QString("%0 <= %1").arg(QChar((char)('a'+nLetter))).arg(nCount);
            else
                strAndClause += QString("%0 = %1").arg(QChar((char)('a'+nLetter))).arg(nCount);

            if (nLetter < 25)
                strAndClause += QString(" AND ");
        }

        strAndClause += QString(") ");

        QSet<QChar> uniqueChars = getUniqueLetters(Letters);
        QSet<QChar>::iterator it = uniqueChars.begin();
        while (it != uniqueChars.end()) {
            QChar ch = *it;
            // Process the character
            strOrClause += QString("%0 > 0").arg(ch);
            // Check if it's the last element
            if (std::next(it) != uniqueChars.end()) {
                strOrClause += QString(" OR ");
            }

            ++it;
        }
        strOrClause += QString(") ");

        QString strSQLStatement = strSelectClause + strWhereClause + strAndClause + strOrClause + strLikeClause + strOrderBy;

        qDebug() << "The SQL Statement: " << strSQLStatement;

        QSqlQuery query(_db);

        if (!query.exec(strSQLStatement)) {
            qDebug() << "Error: Failed to execute query." << query.lastError().text();
            return QSqlQuery();
        }

        return query;
    }

    QSqlQuery FindWordsStartingWith(QChar Letter){
        QSqlQuery query(_db);

        QString queryString = QString("SELECT word FROM words WHERE ").arg(Letter);
        if (!query.exec(queryString)) {
            qDebug() << "Error: Failed to execute query." << query.lastError().text();
            return QSqlQuery();
        }

        return query;
    }

    ~CWordFind(){
        _db.close();
    }

private:
    QSqlDatabase _db;
    QSqlQuery _query;
};


CWordFind wordFind;

ScrabbleWordFinder::ScrabbleWordFinder(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ScrabbleWordFinder) {
    ui->setupUi(this);

    wordFind.OpenDatabase("wordinfo.db");


    ui->btnFind->connect(ui->btnFind,&QPushButton::clicked,[this](){
        QStandardItemModel *model = new QStandardItemModel(this);
        model->setColumnCount(1); // We only need one column for the word

        QString letters = ui->txtLetters->text();
        int length = ui->txtLength->text().toInt();
        QString start = ui->txtStart->text();
        QString contains = ui->txtContains->text();
        QString end = ui->txtEnd->text();

        if ( length == 0 )
            length = 99;

        if ( letters.length() == 0 )
            return;

        QSqlQuery query = wordFind.FindWords(letters,length,start,contains,end);

        while (query.next()) {
            QString word = query.value(0).toString();
            QStandardItem *item = new QStandardItem(word);
            model->appendRow(item);
        }

        // Set the model to the tree view
        ui->trvFound->setModel(model);
    });
}

ScrabbleWordFinder::~ScrabbleWordFinder() {
    delete ui;
}
