#ifndef ARDUINO_H
#define ARDUINO_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QDebug>
#include <QTimer>

class Arduino : public QObject
{
    Q_OBJECT

public:
    explicit Arduino(QObject *parent = nullptr);
    ~Arduino();

    bool connectToArduino(const QString &portName);
    void disconnectFromArduino();
    bool isConnected() const;
    void sendCommand(const QString &command);

    // Get the current stock level (0-100%)
    int getStockLevel() const;

    // Get the current distance measurement in cm
    int getDistance() const;

    // Check if stock is low
    bool isStockLow() const;

signals:
    // Signal emitted when stock data is received
    void stockDataReceived(int level, int distance);

    // Signal emitted when stock level is low
    void stockLevelLow();

    // Signal emitted when Arduino sends any message
    void messageReceived(const QString &message);

    // Signal emitted when connection status changes
    void connectionStatusChanged(bool connected);

public slots:
    // Request current stock level from Arduino
    void requestStockLevel();

    // Start calibration process
    void startCalibration();

private slots:
    void readSerialData();
    void handleError(QSerialPort::SerialPortError error);
    void checkConnection();

private:
    QSerialPort *m_serialPort;
    QTimer *m_connectionTimer;
    bool m_connected;
    int m_stockLevel;
    int m_distance;
    bool m_stockLow;

    void processMessage(const QString &message);
};

#endif // ARDUINO_H
