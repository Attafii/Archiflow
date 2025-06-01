#include "Arduino.h"
#include <QSerialPortInfo>

Arduino::Arduino(QObject *parent) : QObject(parent),
    m_serialPort(new QSerialPort(this)),
    m_connectionTimer(new QTimer(this)),
    m_connected(false),
    m_stockLevel(0),
    m_distance(0),
    m_stockLow(false)
{
    // Connect signals and slots for serial communication
    connect(m_serialPort, &QSerialPort::readyRead, this, &Arduino::readSerialData);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &Arduino::handleError);

    // Setup connection check timer
    m_connectionTimer->setInterval(5000); // Check every 5 seconds
    connect(m_connectionTimer, &QTimer::timeout, this, &Arduino::checkConnection);
}

Arduino::~Arduino()
{
    if (m_connected) {
        disconnectFromArduino();
    }

    delete m_serialPort;
}

bool Arduino::connectToArduino(const QString &portName)
{
    // Close any existing connection
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }

    // Configure serial port
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(QSerialPort::Baud9600);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    // Try to open the serial port
    if (m_serialPort->open(QIODevice::ReadWrite)) {
        m_connected = true;
        m_connectionTimer->start();
        emit connectionStatusChanged(true);

        // Wait for Arduino to initialize
        QThread::msleep(2000);

        // Request initial stock level
        requestStockLevel();

        return true;
    } else {
        m_connected = false;
        emit connectionStatusChanged(false);
        return false;
    }
}

void Arduino::disconnectFromArduino()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }

    m_connected = false;
    m_connectionTimer->stop();
    emit connectionStatusChanged(false);
}

bool Arduino::isConnected() const
{
    return m_connected;
}

void Arduino::sendCommand(const QString &command)
{
    if (m_connected && m_serialPort->isOpen()) {
        m_serialPort->write((command + "\n").toUtf8());
    }
}

int Arduino::getStockLevel() const
{
    return m_stockLevel;
}

int Arduino::getDistance() const
{
    return m_distance;
}

bool Arduino::isStockLow() const
{
    return m_stockLow;
}

void Arduino::requestStockLevel()
{
    sendCommand("GET_STOCK");
}

void Arduino::startCalibration()
{
    sendCommand("CALIBRATE");
}

void Arduino::readSerialData()
{
    while (m_serialPort->canReadLine()) {
        QByteArray data = m_serialPort->readLine().trimmed();
        QString message = QString::fromUtf8(data);

        // Process the received message
        processMessage(message);

        // Emit the raw message signal
        emit messageReceived(message);
    }
}

void Arduino::processMessage(const QString &message)
{
    // Check if it's a stock level update
    if (message.startsWith("STOCK:")) {
        QStringList parts = message.split(":");
        if (parts.size() >= 3) {
            bool ok1, ok2;
            int level = parts[1].toInt(&ok1);
            int distance = parts[2].toInt(&ok2);

            if (ok1 && ok2) {
                m_stockLevel = level;
                m_distance = distance;
                emit stockDataReceived(level, distance);
            }
        }
    }
    // Check if it's a low stock alert
    else if (message == "ALERT:LOW_STOCK") {
        m_stockLow = true;
        emit stockLevelLow();
    }
}

void Arduino::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError && m_connected) {
        // Handle serious errors by disconnecting
        disconnectFromArduino();
    }
}

void Arduino::checkConnection()
{
    if (m_connected) {
        // Send a simple command to check if Arduino is still responding
        requestStockLevel();
    }
}
```

    ## MainWindow Integration Example

            Here's an example of how to integrate the Arduino stock monitoring into a MainWindow class:

```cpp
// In MainWindow.h
#include "Arduino.h"

    class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectButtonClicked();
    void onStockDataReceived(int level, int distance);
    void onStockLevelLow();
    void onConnectionStatusChanged(bool connected);
    void updatePortList();

private:
    Ui::MainWindow *ui;
    Arduino *m_arduino;
    QTimer *m_portUpdateTimer;

    void setupArduinoConnections();
    void populatePortList();
};

// In MainWindow.cpp
void MainWindow::setupArduinoConnections()
{
    m_arduino = new Arduino(this);

    // Connect Arduino signals to slots
    connect(m_arduino, &Arduino::stockDataReceived, this, &MainWindow::onStockDataReceived);
    connect(m_arduino, &Arduino::stockLevelLow, this, &MainWindow::onStockLevelLow);
    connect(m_arduino, &Arduino::connectionStatusChanged, this, &MainWindow::onConnectionStatusChanged);

    // Setup port update timer
    m_portUpdateTimer = new QTimer(this);
    connect(m_portUpdateTimer, &QTimer::timeout, this, &MainWindow::updatePortList);
    m_portUpdateTimer->start(5000); // Update port list every 5 seconds

    // Initial port list population
    populatePortList();
}

void MainWindow::onStockDataReceived(int level, int distance)
{
    // Update UI with stock level information
    ui->stockLevelProgressBar->setValue(level);
    ui->distanceLabel->setText(QString("%1 cm").arg(distance));

    // Update stock status display
    if (m_arduino->isStockLow()) {
        ui->stockStatusLabel->setText("LOW STOCK!");
        ui->stockStatusLabel->setStyleSheet("color: red; font-weight: bold;");
    } else {
        ui->stockStatusLabel->setText("Stock OK");
        ui->stockStatusLabel->setStyleSheet("color: green;");
    }
}

void MainWindow::onStockLevelLow()
{
    // Show alert notification
    QMessageBox::warning(this, "Stock Alert", "Material stock level is low! Please refill soon.");
}

void MainWindow::populatePortList()
{
    ui->portComboBox->clear();

    // Get list of available serial ports
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        ui->portComboBox->addItem(info.portName());
    }
}

void MainWindow::onConnectButtonClicked()
{
    if (m_arduino->isConnected()) {
        m_arduino->disconnectFromArduino();
        ui->connectButton->setText("Connect");
    } else {
        QString portName = ui->portComboBox->currentText();
        if (m_arduino->connectToArduino(portName)) {
            ui->connectButton->setText("Disconnect");
        } else {
            QMessageBox::critical(this, "Connection Error",
                                  "Could not connect to Arduino on port " + portName);
        }
    }
}
