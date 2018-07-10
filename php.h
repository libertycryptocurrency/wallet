#ifndef PHP_H
#define PHP_H

#include <QObject>
#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QHash>
#include <QDir>
#include <QFileInfo>

#include <cmath>



class PHP : public QObject
{
    Q_OBJECT
public:
    explicit PHP(QObject *parent = 0);

    QString version();
    QString exec(QString script, QString arg1 = "", QString arg2 = "", QString arg3 = "");
    void execAsync(QString script, QString arg1 = "", QString arg2 = "", QString arg3 = "");
    int initialize();

    QString wallet(QString func, QString param1 = "", QString param2 = "");
    void walletAsync(QString func, QString param1 = "", QString param2 = "");

    bool create(QString password);
    QString newaddr(QString password);
    QStringList listaddr(QString password);
    void balance(QString password);
    bool send(QString password, QString from, QString to, QString amount);
    void transactionsByDate(QString password);
    QString importaddr(QString password, QString privk);
    QString exportaddr(QString password, QString addr);
    void synchronize();

    static quint32 fee(quint32 amount);

private:
    QProcess *process;
    QString inProcess;

public slots:
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

signals:
    void signalSynchronize(QStringList list);
    void signalBalance(QHash<QString, QString> hash);
    void signalTransactionsByDate(QStringList list);

};

#endif // PHP_H
