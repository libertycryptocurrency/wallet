#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QtWebKit>
#include <QWebView>
#include <QDebug>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QProcess>
#include <QHash>
#include <QHashIterator>
#include <QFileInfo>
#include <QDateTime>
#include <QApplication>
#include <QDesktopServices>

#include <php.h>


class MainWindow : public QWebView
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //QString path;
    //QString appdata;
    QHash<QString, QString> balance;
    QTimer *timer;

    static QString path();
    static QString appdata();

private:
    QWebPage *webpage;
    QWebFrame *webframe;
    QWebElement webelement;
    PHP *php;
    QString password;
    QString afterLoad;

    void testPHP();

public slots:
    void status(const QString &text);
    void timeout();
    void loadFinished(bool);
    void signalSynchronize(QStringList list);
    void signalBalance(QHash<QString, QString> hash);
    void signalTransactionsByDate(QStringList list);

signals:

};

#endif // MAINWINDOW_H
