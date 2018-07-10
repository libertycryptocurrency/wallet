
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QWebView(parent)
{

#ifdef QT_DEBUG
    setWindowTitle(tr("Liberty Wallet (Debug)"));
#else
    setWindowTitle(tr("Liberty Wallet"));
#endif

    setWindowIcon(QIcon(":/liberty32.png"));
    setFixedSize(QSize(360, 640));
    setStyleSheet("color: #000; background-color: #039;");

    php = new PHP(this);
    timer = new QTimer(this);

    webpage = page();

    connect(this, SIGNAL(statusBarMessage(QString)), this, SLOT(status(QString)));
    connect(webpage, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));

    connect(php, SIGNAL(signalSynchronize(QStringList)), this, SLOT(signalSynchronize(QStringList)));
    connect(php, SIGNAL(signalBalance(QHash<QString, QString>)), this, SLOT(signalBalance(QHash<QString, QString>)));
    connect(php, SIGNAL(signalTransactionsByDate(QStringList)), this, SLOT(signalTransactionsByDate(QStringList)));

    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));

    // php.ini initializacion
    php->initialize();


    // UNCOMMENT BEFORE DEBUG
    QFileInfo ex(this->appdata() + "/wallet.ini");
    if(ex.exists()){
        load(QUrl(this->appdata() + "/login.html"));
        afterLoad = "login";
    } else {
        load(QUrl(this->appdata() + "/create.html"));
        afterLoad = "create";
    }

    // Wallet Debug more fast
    /*
    this->password = "12345678";
    load(QUrl(this->appdata() + "/wallet.html"));
    afterLoad = "wallet";
    */

    qDebug() << php->version();
}


MainWindow::~MainWindow()
{
}








void MainWindow::loadFinished(bool status)
{
    Q_UNUSED(status);
    //qDebug() << afterLoad + " was loaded";

    setStyleSheet("color: #000; background-color: #FFF;");

    webframe = webpage->mainFrame();
    webelement = webframe->documentElement();

    if(afterLoad == "wallet") {
        php->balance(this->password);
        afterLoad = "";
        if(!timer->isActive()) {
            timer->start(120 * 1000);
        }
        return;
    }
}





void MainWindow::timeout()
{
    qDebug() << "timeout!";
    //timer->stop();
    php->balance(this->password);
}




void MainWindow::signalSynchronize(QStringList list)
{
    Q_UNUSED(list);

    // List of nodes
    if(list.size() > 0) {
        qDebug() << list.at(0);
    }

    load(QUrl(this->appdata() + "/wallet.html"));
    afterLoad = "wallet";
}




void MainWindow::signalBalance(QHash<QString, QString> hash)
{
    this->balance = hash;
    //qDebug() << this->balance["total"];

    // Set the Balance
    webframe = webpage->mainFrame();
    webelement = webframe->documentElement();
    QWebElement balance = webelement.findFirst("div#balance");
    //qDebug() << div.toPlainText();
    //div.setPlainText("Este texto es generado por QT.");

    if(hash.size() == 0) return;

    if(hash["total"] == "") {
        balance.setPlainText("No Connection");
        return;
    } else {
        balance.setPlainText(QString("%L1").arg(hash["total"].toDouble(), 0, 'f', 2, '0'));
    }


    // My Addresses with Amount
    QString addr = "";
    QWebElement tbody = webelement.findFirst("tbody#frameMyAddr");

    if(hash.size() < 1) return;
    qDebug() << hash;

    QHashIterator<QString, QString> i(hash);
    while (i.hasNext()) {
        i.next();
        if(i.key() == "total") continue;
        addr.append("<tr><td>" + i.key() + "</td><td class=\"right\">" + QString("%L1").arg(i.value().toDouble(), 0, 'f', 2, '0') + "</td></tr>");
    }

    tbody.setInnerXml(addr);


    // Call Transactions By Date
    php->transactionsByDate(this->password);
}





void MainWindow::signalTransactionsByDate(QStringList list)
{
    if(list.size() == 0) return;
    qDebug() << list;

    // Set the transactions
    webframe = webpage->mainFrame();
    webelement = webframe->documentElement();
    QWebElement tx = webelement.findFirst("tbody#frameTransactions");

    QStringList line;
    QString txHTML;
    QDateTime dt;
    QString date;
    QString from;
    QString to;

    for(int a=0; a < (list.size() - 1); a++) {
        line = list.at(a).split(" ");

        if(line.at(9) == "in") {
            txHTML.append("<tr class=\"in\">");
        } else {
            txHTML.append("<tr class=\"out\">");
        }

        // Date
        dt = QDateTime::fromTime_t(line.at(5).toInt());
        date = dt.toString("MM/dd/yy");
        txHTML.append("<td>" + date + "</td>");

        // From
        from = line.at(1);
        from.remove(7, from.size()-1);
        txHTML.append("<td title=\"" + line.at(1) + "\">" + from + "..</td>");

        // To
        to = line.at(2);
        to.remove(7, to.size()-1);
        txHTML.append("<td title=\"" + line.at(2) + "\">" + to + "..</td>");

        // Amount
        txHTML.append("<td class=\"right\">" + QString("%L1").arg(line.at(3).toDouble(), 0, 'f', 2, '0')  + "</td>");


        txHTML.append("</tr>");
    }

    tx.setInnerXml(txHTML);
}




void MainWindow::status(const QString &text)
{
    webframe = webpage->mainFrame();
    webelement = webframe->documentElement();

    if(text == "") return;
    qDebug() << text;

    QStringList data = text.split(" ");

    if(data.at(0) == "password") {
        php->create(data.at(1));
        load(QUrl(this->appdata() + "/login.html"));
        return;
    }

    if(data.at(0) == "login") {
        this->password = data.at(1);
        QStringList listaddr = php->listaddr(this->password);

        if(listaddr.at(0) == "wrong") {
            webelement.evaluateJavaScript("passwordWrong();");
            return;
        }

        webelement.evaluateJavaScript("toggleCreate();");
        php->synchronize();
        return;
    }

    if(data.at(0) == "send") {

        // Webkit error: https://bugs.webkit.org/show_bug.cgi?id=32865 (3 days to find..)
        // Cannot return input value via webelement.attribute("value");
        QString sendTo = webelement.evaluateJavaScript("$('#sendTo').val();").toString();
        QString sendAmount = webelement.evaluateJavaScript("$('#sendAmount').val();").toString();

        if(sendTo == "") return;
        if(sendAmount == "") return;

        QApplication::setOverrideCursor(Qt::WaitCursor);

        quint32 bal = (quint32)round(this->balance["total"].toFloat() * 100);
        quint32 amount = (quint32)round(sendAmount.toFloat() * 100);

        if(bal < amount) {
            qDebug() << "Insufficient Funds";
            QApplication::restoreOverrideCursor();
            webelement.evaluateJavaScript("sendNoFunds();");
            return;
        }

        quint32 remain = amount;
        quint32 rounded;
        quint32 ivalue;

        QHashIterator<QString, QString> i(balance);

        while (i.hasNext()) {
            i.next();
            //qDebug() << i.key() << ": " << i.value().toFloat();

            ivalue = (quint32)round(i.value().toFloat() * 100);

            if(i.key() == "total") continue;
            if(ivalue == 0) continue;

            if(ivalue >= remain) {
                remain = remain - PHP::fee(remain);
                php->send(this->password, i.key(), sendTo, QString::number((float)remain / 100));
                //qDebug() << i.key() << " " << sendTo << " " <<  QString::number((float)remain / 100);
                remain = 0;
                break;
            } else {
                rounded = ivalue - PHP::fee(ivalue);
                php->send(this->password, i.key(), sendTo, QString::number((float)rounded / 100));
                //qDebug() << i.key() << " " << sendTo << " " <<  QString::number((float)rounded / 100);
                remain = remain - ivalue;
            }
        }

        QApplication::restoreOverrideCursor();

        webelement.evaluateJavaScript("sendInProgress();");

        return;
    }
}





QString MainWindow::path()
{
    QString path;
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    path = QDir::current().absolutePath();
    return path;
}





QString MainWindow::appdata()
{
    //qDebug() << QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#ifdef QT_DEBUG
    return MainWindow::path() + "/appdata";
#else
    QDir::setCurrent(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
    return QDir::current().absolutePath() + "/LibertyWallet-1.0";
#endif
}





void MainWindow::testPHP()
{
    // synchronize
    /*
    QStringList list = php->synchronize();

    for (int i = 0; i < list.size(); ++i) {
        if(list.at(i) == "") continue;
        qDebug() << i << ") " << list.at(i);
    }
    */

    // Create
    //qDebug() << php->create("1234");


    // Newaddr
    //qDebug() << php->newaddr("1234");

    // Listaddr
    //qDebug() << php->listaddr("1234");

    // Balance
    /*
    QHash<QString, QString> hash;

    hash = php->balance("1234");

    QHashIterator<QString, QString> i(hash);
    while (i.hasNext()) {
        i.next();
        qDebug() << i.key() << ": " << i.value().toFloat();
    }
    */

    // Send
    /*
    qDebug() << php->send("1234",
                          "La58pagQBdCDhQFBnvvqt2jVAGmWHozL1h",
                          "LfetwJqzg7Ckxg3HaYqmpgR3v6mvQK2P7B",
                          "1.00");
    */

    // qDebug() << php->transactions("1234", "La58pagQBdCDhQFBnvvqt2jVAGmWHozL1h");

    // qDebug() << php->exportaddr("1234", "La58pagQBdCDhQFBnvvqt2jVAGmWHozL1h");

    // qDebug() << php->importaddr("1234", "T4m3R1THtX2nUgG2PTJamVXe9xm1WtybdBPqKdXuGMhyCZtD8HV4");
}




