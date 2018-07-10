
#include "php.h"
#include "mainwindow.h"



PHP::PHP(QObject *parent) :
    QObject(parent)
{
    process = new QProcess(this);
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(processFinished(int,QProcess::ExitStatus)));
}



// Events and Slots

void PHP::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString ret;

    if(exitStatus == QProcess::NormalExit) {
        qDebug() << "inProcess: " << inProcess;

        if(inProcess == "synchronize") {
            ret = process->readAllStandardOutput();
            if(ret == "no node") return;
            QStringList list = ret.split(" ");
            emit this->signalSynchronize(list);
            return;
        }

        if(inProcess == "balance") {
            QString ret;
            QStringList list;
            QStringList balance;
            QHash<QString, QString> hash;

            ret = process->readAllStandardOutput();

            list = ret.split("\n");

            if(list.count() == 0) return;

            for (int i = 0; i < list.size(); ++i) {
                balance = list.at(i).split("=");

                if(balance.size() == 0) continue;

                if(balance.at(0) == "balance") {
                    hash["total"] = balance.at(1);
                } else {
                    hash[balance.at(0)] = balance.at(1);
                }
            }

            emit this->signalBalance(hash);
            return;
        }

        if(inProcess == "txbydate") {
            QString ret;
            QStringList list;
            ret = process->readAllStandardOutput();
            list = ret.split("\n");
            emit this->signalTransactionsByDate(list);
            return;
        }
    } else {
        qDebug() << "Crash: " << exitCode;
    }
}




// Implementations

QString PHP::version()
{
    QString program;
    QFileInfo check;

    check.setFile(MainWindow::appdata() + "/PHP7");
    if(check.exists() && check.isDir()) {
        program = MainWindow::appdata() + "/PHP7/php.exe";
    }

    check.setFile(MainWindow::appdata() + "/PHP5");
    if(check.exists() && check.isDir()) {
        program = MainWindow::appdata() + "/PHP5/php.exe";
    }

    QStringList arguments;
    arguments << "-v";

    process->start(program, arguments);
    process->waitForFinished();
    return process->readAllStandardOutput();
}




QString PHP::exec(QString script, QString arg1, QString arg2, QString arg3)
{

#if defined Q_WS_WIN
    QString program;
    QFileInfo check;

    check.setFile(MainWindow::appdata() + "/PHP7");
    if(check.exists() && check.isDir()) {
        program = MainWindow::appdata() + "/PHP7/php.exe";
    }

    check.setFile(MainWindow::appdata() + "/PHP5");
    if(check.exists() && check.isDir()) {
        program = MainWindow::appdata() + "/PHP5/php.exe";
    }
#else
    QString program = "php";
#endif

    QStringList arguments;
    arguments << MainWindow::appdata() + "/" + script << arg1 << arg2 << arg3;

    process->start(program, arguments);
    process->waitForFinished();
    return process->readAllStandardOutput();
}




void PHP::execAsync(QString script, QString arg1, QString arg2, QString arg3)
{

#if defined Q_WS_WIN
    QString program;
    QFileInfo check;

    check.setFile(MainWindow::appdata() + "/PHP7");
    if(check.exists() && check.isDir()) {
        program = MainWindow::appdata() + "/PHP7/php.exe";
    }

    check.setFile(MainWindow::appdata() + "/PHP5");
    if(check.exists() && check.isDir()) {
        program = MainWindow::appdata() + "/PHP5/php.exe";
    }
#else
    QString program = "php";
#endif

    QStringList arguments;
    arguments << MainWindow::appdata() + "/" + script << arg1 << arg2 << arg3;

    process->start(program, arguments);
}





int PHP::initialize()
{
    QFileInfo check(MainWindow::appdata() + "/PHP7");

    if(check.exists() && check.isDir()) {

        check.setFile(MainWindow::appdata() + "/PHP7/php.ini");
        if (check.exists() && check.isFile()) {
            qDebug() << "php.ini is ok.";
            return 0;
        }

        check.setFile(MainWindow::appdata() + "/PHP7/php.ini.development");
        if (check.exists() && check.isFile()) {
            qDebug() << "php.ini not set.";

            QFile file(MainWindow::appdata() + "/PHP7/php.ini.development");
            file.copy(MainWindow::appdata() + "/PHP7/php.ini");
            file.close();

            file.setFileName(MainWindow::appdata() + "/PHP7/php.ini");
            if (file.open(QIODevice::Append | QIODevice::Text))
            {
                QTextStream stream(&file);

                stream << "include_path = \"" + MainWindow::appdata() + "/PHP7/PEAR\"" << endl;
                stream << "extension_dir = \"" + MainWindow::appdata() + "/PHP7/ext\"" << endl;
                stream << "[curl]" << endl;
                stream << "curl.cainfo = \"" + MainWindow::appdata() + "/PHP7/extras/curl-ca-bundle.crt\"" << endl;

                file.close();
            }
        }
        return 1;
    }

    return 0;
}





QString PHP::wallet(QString func, QString param1, QString param2)
{
    return this->exec("iwallet.php", func, param1, param2);
}




void PHP::walletAsync(QString func, QString param1, QString param2)
{
    this->execAsync("iwallet.php", func, param1, param2);
}






// Wallet Specific

bool PHP::create(QString password)
{
    QString ret;
    inProcess = "";
    ret = this->wallet("create", password);
    if(ret == "exists") {
        return false;
    }
    if(ret == "created") {
        return true;
    }

    return false;
}




QString PHP::newaddr(QString password)
{
    QString ret;
    inProcess = "";
    ret = this->wallet("newaddr", password);
    if(ret == "wrong") {
        return "false";
    } else {
        return ret;
    }
    return "false";
}



QStringList PHP::listaddr(QString password)
{
    QString ret;
    inProcess = "";
    ret = this->wallet("listaddr", password);
    return ret.split(" ");
}




/*
// Use iterator
QHashIterator<QString, QString> i(hash);
while (i.hasNext()) {
    i.next();
    qDebug() << i.key() << ": " << i.value().toFloat();
}
*/
void PHP::balance(QString password)
{
    this->walletAsync("balance", password);
    inProcess = "balance";
}





bool PHP::send(QString password, QString from, QString to, QString amount)
{
    QString ret;
    inProcess = "";
    ret = this->wallet("send", password, from + " " + to + " " + amount);
    if(ret == "inprogress") {
        return true;
    } else {
        return false;
    }
    return false;
}




void PHP::transactionsByDate(QString password)
{
    this->walletAsync("transactionsbydate", password);
    inProcess = "txbydate";
}





QString PHP::importaddr(QString password, QString privk)
{
    QString ret;
    inProcess = "";
    ret = this->wallet("importaddr", password, privk);
    return ret;
}




QString PHP::exportaddr(QString password, QString addr)
{
    QString ret;
    inProcess = "";
    ret = this->wallet("exportaddr", password, addr);
    return ret;
}




void PHP::synchronize()
{
    this->walletAsync("synchronize");
    inProcess = "synchronize";
}




quint32 PHP::fee(quint32 amount)
{
    quint32 fee;

    if(amount >= 1000) {
        fee = (quint32)round(amount * 0.001);
        return fee;
    }

    if(amount < 1000) {
        return 1;
    }
    return 0;
}



