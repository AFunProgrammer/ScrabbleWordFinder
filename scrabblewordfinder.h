#ifndef ScrabbleWordFinder_H
#define ScrabbleWordFinder_H

#include <QtSystemDetection>
#include <QApplication>
#include <QMainWindow>


QT_BEGIN_NAMESPACE
namespace Ui {
class ScrabbleWordFinder;
}
QT_END_NAMESPACE

class ScrabbleWordFinder : public QMainWindow
{
    Q_OBJECT

public:
    ScrabbleWordFinder(QWidget *parent = nullptr);
    ~ScrabbleWordFinder();

private:
    Ui::ScrabbleWordFinder *ui;
};
#endif // ScrabbleWordFinder_H
