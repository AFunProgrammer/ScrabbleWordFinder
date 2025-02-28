#include "wordfinder.h"
#include "ui_wordfinder.h"
#include <QAction>
#include <QtGlobal>
#include <QDir>
#include <QErrorMessage>
#include <QResource>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QException>

//Android Error Checking Because I cannot connect the debugger to the emulator
// no matter what I try to do...
QList<QString> _errorMessages;

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
            qDebug() << "An exception occurred." << e.what();
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
        int nEndLen = (Length != 99 ? Length : (Letters.length() > 0 ? Letters.length() : 99));

        QString strSelectClause = "SELECT word FROM words ";
        QString strWhereClause = QString("WHERE length BETWEEN %0 AND %1 ").arg(nStartLen).arg(nEndLen);
        QString strAndClause = "AND (";
        QString strOrClause = "AND (";
        QString strLikeClause = QString(" AND word LIKE '%0%%%1%%%2' ").arg(Start).arg(Contains).arg(End);
        QString strOrderBy = " ORDER BY length DESC,word ASC ";
        QString strLimitClause = " LIMIT 200 ";


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



        QString strSQLStatement = "";

        if ( Letters.length() > 0 ){
            strSQLStatement = strSelectClause + strWhereClause + strAndClause + strOrClause + strLikeClause + strOrderBy + QString(";");
        } else {
            strSQLStatement = strSelectClause + strWhereClause + strLikeClause + strOrderBy + strLimitClause + QString(";");
        }

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


WordFinder::WordFinder(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::WordFinder) {
    ui->setupUi(this);

    // Get the current default font size
    QFont font = this->font();

#if defined(Q_OS_ANDROID)
    QString localStorage = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = localStorage + "/wordinfo.db";
    //_errorMessages.append( QString("Received writeable location: " + localStorage) );
    if(!QFile::exists(filePath)){
        if ( !QFile::copy("assets:/wordinfo.db", localStorage + "/wordinfo.db")){
            qWarning() << "Could not copy sql lite wordinfo.db file to read/write destination";
            //QString failedToCopy = QString("Couldn't copy to: " + localStorage + "/wordinfo.db");
            //_errorMessages.append( failedToCopy );
            //QStandardItemModel *model = new QStandardItemModel(this);
            //model->setColumnCount(1); // We only need one column for the word

            //for( QString error: _errorMessages ) {
            //    QStandardItem *item = new QStandardItem(error);
            //    model->appendRow(item);
            //}
            //ui->trvFound->setModel(model);
            return;
        }
    }
    wordFind.OpenDatabase(localStorage + "/wordinfo.db");
    font.setPointSize(font.pointSize() + 10);

    // Load the qlistview.css from the qrc resource
    QFile file(":/styles/qlistcss");  // Path to the resource alias
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream ts(&file);
        QString css = ts.readAll();  // Read the content as a string

        QColor highlight = ui->lstFound->palette().highlight().color();
        QString highlightCss = QString("background: rgb(%0,%1,%2);").arg(highlight.red()).arg(highlight.green()).arg(highlight.blue());
        QString highlightCssDark = QString("background: rgb(%0,%1,%2);").arg(highlight.red()/1.5).arg(highlight.green()/1.5).arg(highlight.blue()/1.5);

        css = css.replace("background: rgb(0,0,0);", highlightCss);
        css = css.replace("background: rgb(1,1,1);", highlightCssDark);

        ui->lstFound->setStyleSheet(css); // Apply the CSS to trvFound
    }
#else //Desktop OSes
    wordFind.OpenDatabase("wordinfo.db");
    font.setPointSize(font.pointSize() + 6);
#endif
    // Set the Desired Font Size
    this->setFont(font);

    // QT_BUG: BUG: BUG: there's a bug with setting the QList stylesheet and setting the font size
    //  So I need to set the font size seperately after I've set the stylesheet
    QFont lstFont = ui->lstFound->font();
    lstFont.setPointSize(font.pointSize());
    ui->lstFound->setFont(lstFont);

    ui->btnFind->connect(ui->btnFind,&QPushButton::clicked,[this](){
        QString letters = ui->txtLetters->text().toLower();
        int length = ui->txtLength->text().toInt();
        QString start = ui->txtStart->text().toLower();
        QString contains = ui->txtContains->text().toLower();
        QString end = ui->txtEnd->text().toLower();

        //clear tree view
        ui->lstFound->clear();

        if ( length == 0 )
            length = 99;

        if ( letters.length() == 0 && start.length() == 0 && contains.length() == 0 && end.length() == 0 && length == 0)
            return;

        QSqlQuery query = wordFind.FindWords(letters,length,start,contains,end);
        QList<QString> foundWords;
        while (query.next()) {
            foundWords.append(query.value(0).toString());
        }

        ui->lstFound->addItems(foundWords);
    });

    ui->btnExit->connect(ui->btnExit, &QPushButton::clicked, qApp, &QCoreApplication::quit);
}

WordFinder::~WordFinder() {
    delete ui;
}
