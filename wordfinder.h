#ifndef WordFinder_H
#define WordFinder_H

#include <QApplication>
#include <QMainWindow>


QT_BEGIN_NAMESPACE
namespace Ui {
class WordFinder;
}
QT_END_NAMESPACE

class WordFinder : public QMainWindow
{
    Q_OBJECT

public:
    WordFinder(QWidget *parent = nullptr);
    ~WordFinder();

private:
    Ui::WordFinder *ui;
};
#endif // WordFinder_H
