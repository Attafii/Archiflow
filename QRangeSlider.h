#ifndef QRANGESLIDER_H
#define QRANGESLIDER_H

#include <QWidget>
#include <QSlider>

/**
 * @brief The QRangeSlider class provides a slider widget with two handles for selecting a range of values.
 * 
 * This is a custom implementation since QRangeSlider is not part of the standard Qt library.
 * It uses two QSlider widgets to create a range selection control.
 */
class QRangeSlider : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int lowerValue READ lowerValue WRITE setLowerValue NOTIFY lowerValueChanged)
    Q_PROPERTY(int upperValue READ upperValue WRITE setUpperValue NOTIFY upperValueChanged)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)

public:
    explicit QRangeSlider(QWidget *parent = nullptr);
    explicit QRangeSlider(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~QRangeSlider();

    int minimum() const;
    int maximum() const;
    int lowerValue() const;
    int upperValue() const;
    Qt::Orientation orientation() const;

    void setRange(int minimum, int maximum);

public slots:
    void setMinimum(int minimum);
    void setMaximum(int maximum);
    void setLowerValue(int lowerValue);
    void setUpperValue(int upperValue);
    void setOrientation(Qt::Orientation orientation);

signals:
    void rangeChanged(int minimum, int maximum);
    void lowerValueChanged(int lowerValue);
    void upperValueChanged(int upperValue);

private:
    QSlider *m_lowerSlider;
    QSlider *m_upperSlider;
    int m_minimum;
    int m_maximum;
    int m_lowerValue;
    int m_upperValue;
    Qt::Orientation m_orientation;

    void initialize();
    void updateSliderPositions();
};

#endif // QRANGESLIDER_H