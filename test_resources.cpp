#include <QApplication>
#include <QLabel>
#include <QPixmap>
#include <QDebug>
#include <QDir>
#include <QResource>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== RESOURCE TEST ===";
    qDebug() << "Available resources in :/:" << QDir(":/").entryList();
    qDebug() << "Available resources in :/icons:" << QDir(":/icons").entryList();
    
    qDebug() << "QResource exists for :/icons/ArchiFlow.svg:" << QResource(":/icons/ArchiFlow.svg").isValid();
    qDebug() << "QResource exists for :/icons/a.png:" << QResource(":/icons/a.png").isValid();
    
    QPixmap logo1(":/icons/ArchiFlow.svg");
    QPixmap logo2(":/icons/a.png");
    
    qDebug() << "Logo1 (SVG) isNull:" << logo1.isNull() << "size:" << logo1.size();
    qDebug() << "Logo2 (PNG) isNull:" << logo2.isNull() << "size:" << logo2.size();
    
    QLabel label;
    if (!logo2.isNull()) {
        label.setPixmap(logo2);
        label.show();
        qDebug() << "Showing PNG logo";
    } else {
        label.setText("No logo loaded");
        label.show();
        qDebug() << "Showing text fallback";
    }
    
    return app.exec();
}
