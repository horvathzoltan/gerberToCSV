#include <QCommandLineParser>
#include <QCoreApplication>
#include "common/logger/log.h"
#include "common/helper/signalhelper/signalhelper.h"
#include "common/helper/CommandLineParserHelper/commandlineparserhelper.h"
#include "common/coreappworker/coreappworker.h"
#include "work1.h"


// gerberToCSV.exe -i Other.gbr -o insole_42.csv

int main(int argc, char *argv[])
{
    com::helper::SignalHelper::setShutDownSignal(com::helper::SignalHelper::SIGINT_); // shut down on ctrl-c
    com::helper::SignalHelper::setShutDownSignal(com::helper::SignalHelper::SIGTERM_); // shut down on killall

    zInfo(QStringLiteral("started"));

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("test1"));

    QCommandLineParser parser;

    parser.setApplicationDescription(QStringLiteral("command line test1 app."));
    parser.addHelpOption();
    parser.addVersionOption();  

    const QString OPTION_IN = QStringLiteral("input");
    const QString OPTION_OUT = QStringLiteral("output");
    const QString OPTION_BACKUP = QStringLiteral("backup");
    const QString OPTION_SIZE = QStringLiteral("size");

    com::helper::CommandLineParserHelper::addOption(&parser, OPTION_IN, QStringLiteral("gerber file as input"));    
    com::helper::CommandLineParserHelper::addOption(&parser, OPTION_OUT, QStringLiteral("csv file as output"));
    com::helper::CommandLineParserHelper::addOption(&parser, OPTION_SIZE, QStringLiteral("insole size"));
    com::helper::CommandLineParserHelper::addOptionBool(&parser, OPTION_BACKUP, QStringLiteral("set if backup is needed"));

    parser.process(app);

    // statikus, számítunk arra, hogy van
    Work1::params.inFile = parser.value(OPTION_IN);
    Work1::params.outFile = parser.value(OPTION_OUT);
    Work1::params.insoleSize = parser.value(OPTION_SIZE);
    Work1::params.isBackup = parser.isSet(OPTION_BACKUP);

    com::CoreAppWorker c(Work1::doWork,&app, &parser);
    int a = c.run();

    //int a = Work1::doWork();

    auto e = QCoreApplication::exec();
    return e;
}
