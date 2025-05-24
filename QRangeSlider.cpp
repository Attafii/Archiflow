#include "QRangeSlider.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

QRangeSlider::QRangeSlider(QWidget *parent)
    : QWidget(parent)
    , m_lowerSlider(nullptr)
    , m_upperSlider(nullptr)
    , m_minimum(0)
    , m_maximum(100)
    , m_lowerValue(0)
    , m_upperValue(100)
    , m_orientation(Qt::Horizontal)
{
    initialize();
}

QRangeSlider::QRangeSlider(Qt::Orientation orientation, QWidget *parent)
    : QWidget(parent)
    , m_lowerSlider(nullptr)
    , m_upperSlider(nullptr)
    , m_minimum(0)
    , m_maximum(100)
    , m_lowerValue(0)
    , m_upperValue(100)
    , m_orientation(orientation)
{
    initialize();
}

QRangeSlider::~QRangeSlider()
{
    // QObject parent-child relationship will handle deletion
}

void QRangeSlider::initialize()
{
    m_lowerSlider = new QSlider(m_orientation, this);
    m_upperSlider = new QSlider(m_orientation, this);
    
    m_lowerSlider->setRange(m_minimum, m_maximum);
    m_upperSlider->setRange(m_minimum, m_maximum);
    
    m_lowerSlider->setValue(m_lowerValue);
    m_upperSlider->setValue(m_upperValue);
    
    // Set up layout based on orientation
    if (m_orientation == Qt::Horizontal) {
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_lowerSlider);
        layout->addWidget(m_upperSlider);
    } else {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_upperSlider);
        layout->addWidget(m_lowerSlider);
    }
    
    // Connect signals
    connect(m_lowerSlider, &QSlider::valueChanged, this, [this](int value) {
        if (value > m_upperSlider->value()) {
            m_lowerSlider->setValue(m_upperSlider->value());
            return;
        }
        m_lowerValue = value;
        emit lowerValueChanged(m_lowerValue);
    });
    
    connect(m_upperSlider, &QSlider::valueChanged, this, [this](int value) {
        if (value < m_lowerSlider->value()) {
            m_upperSlider->setValue(m_lowerSlider->value());
            return;
        }
        m_upperValue = value;
        emit upperValueChanged(m_upperValue);
    });
}

int QRangeSlider::minimum() const
{
    return m_minimum;
}

int QRangeSlider::maximum() const
{
    return m_maximum;
}

int QRangeSlider::lowerValue() const
{
    return m_lowerValue;
}

int QRangeSlider::upperValue() const
{
    return m_upperValue;
}

Qt::Orientation QRangeSlider::orientation() const
{
    return m_orientation;
}

void QRangeSlider::setRange(int minimum, int maximum)
{
    if (minimum > maximum) {
        qSwap(minimum, maximum);
    }
    
    m_minimum = minimum;
    m_maximum = maximum;
    
    m_lowerSlider->setRange(minimum, maximum);
    m_upperSlider->setRange(minimum, maximum);
    
    emit rangeChanged(m_minimum, m_maximum);
}

void QRangeSlider::setMinimum(int minimum)
{
    setRange(minimum, m_maximum);
}

void QRangeSlider::setMaximum(int maximum)
{
    setRange(m_minimum, maximum);
}

void QRangeSlider::setLowerValue(int lowerValue)
{
    if (lowerValue > m_upperValue) {
        lowerValue = m_upperValue;
    }
    
    m_lowerValue = lowerValue;
    m_lowerSlider->setValue(lowerValue);
}

void QRangeSlider::setUpperValue(int upperValue)
{
    if (upperValue < m_lowerValue) {
        upperValue = m_lowerValue;
    }
    
    m_upperValue = upperValue;
    m_upperSlider->setValue(upperValue);
}

void QRangeSlider::setOrientation(Qt::Orientation orientation)
{
    if (m_orientation == orientation) {
        return;
    }
    
    m_orientation = orientation;
    
    // We need to recreate the layout and sliders for orientation change
    delete m_lowerSlider;
    delete m_upperSlider;
    
    // Delete the old layout
    if (layout()) {
        QLayoutItem *item;
        while ((item = layout()->takeAt(0)) != nullptr) {
            delete item;
        }
        delete layout();
    }
    
    // Reinitialize with new orientation
    initialize();
    
    updateSliderPositions();
}

void QRangeSlider::updateSliderPositions()
{
    m_lowerSlider->setValue(m_lowerValue);
    m_upperSlider->setValue(m_upperValue);
}