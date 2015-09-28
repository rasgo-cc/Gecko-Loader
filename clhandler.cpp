#include "clhandler.h"
#include <QTextStream>

QTextStream cout(stdout, QIODevice::WriteOnly);

void CLHandler::run()
{
    connect(&loader, SIGNAL(output(QString)), this, SLOT(log(QString)));
    cout << "> Connecting to port " << portName;
    cout.flush();

    if(bootPol == "1")
        loader.setBootEnablePolarity(true);
    else
        loader.setBootEnablePolarity(false);
    if(loader.open(portName))
       loader.upload(filePath);

    cout << "\n";
    cout.flush();

    emit done();
}

void CLHandler::log(const QString &message)
{
    static bool firstPercent = true;
    if(message.contains("/"))
    {
        if(firstPercent)
        {
            cout << "\n";
            firstPercent = false;
        }
        cout << message << "\r";

    }
    else
        cout << "\n> " << message;

    cout.flush();
}
