#include "stylemanager.h"
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QStyle>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

StyleManager::StyleManager(QObject *parent)
    : QObject(parent)
{
}

void StyleManager::applyArchiFlowTheme(QApplication *app)
{
    // Load the main stylesheet
    QFile styleFile(":/styles/main.qss");
    if (styleFile.open(QIODevice::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        app->setStyleSheet(styleSheet);
        qDebug() << "ArchiFlow theme applied successfully";
    } else {
        qWarning() << "Failed to load ArchiFlow stylesheet";
    }
    
    setupColorPalette(app);
}

void StyleManager::applyMaterialDesign(QWidget *widget)
{
    if (!widget) return;
    
    // Apply material design principles
    widget->setProperty("materialDesign", true);
    
    // Add subtle elevation
    addElevation(widget, 1);
    
    // Make corners rounded
    widget->setStyleSheet(widget->styleSheet() + 
                         "border-radius: 8px; "
                         "background-color: white;");
}

void StyleManager::addElevation(QWidget *widget, int level)
{
    if (!widget || level < 0) return;
    
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(widget);
    
    // Configure shadow based on elevation level
    switch (level) {
    case 0:
        // No shadow
        widget->setGraphicsEffect(nullptr);
        return;
    case 1:
        shadow->setBlurRadius(4);
        shadow->setOffset(0, 2);
        shadow->setColor(QColor(0, 0, 0, 50));
        break;
    case 2:
        shadow->setBlurRadius(8);
        shadow->setOffset(0, 4);
        shadow->setColor(QColor(0, 0, 0, 75));
        break;
    case 3:
        shadow->setBlurRadius(12);
        shadow->setOffset(0, 6);
        shadow->setColor(QColor(0, 0, 0, 100));
        break;
    case 4:
        shadow->setBlurRadius(16);
        shadow->setOffset(0, 8);
        shadow->setColor(QColor(0, 0, 0, 125));
        break;
    default:
        shadow->setBlurRadius(20);
        shadow->setOffset(0, 10);
        shadow->setColor(QColor(0, 0, 0, 150));
        break;
    }
    
    widget->setGraphicsEffect(shadow);
}

void StyleManager::addRippleEffect(QPushButton *button)
{
    if (!button) return;
    
    // Install event filter for ripple effect
    button->installEventFilter(new QObject(button));
    
    // Store original stylesheet
    button->setProperty("originalStyle", button->styleSheet());
    
    // Connect to button events for ripple animation
    QObject::connect(button, &QPushButton::pressed, [button]() {
        // Ripple effect on press
        QPropertyAnimation *animation = new QPropertyAnimation(button, "styleSheet");
        animation->setDuration(200);
        animation->setStartValue(button->property("originalStyle").toString());
        animation->setEndValue(button->property("originalStyle").toString() + 
                              "background-color: rgba(227, 198, 176, 0.3);");
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    });
    
    QObject::connect(button, &QPushButton::released, [button]() {
        // Return to normal state
        QPropertyAnimation *animation = new QPropertyAnimation(button, "styleSheet");
        animation->setDuration(300);
        animation->setStartValue(button->styleSheet());
        animation->setEndValue(button->property("originalStyle").toString());
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

QPropertyAnimation* StyleManager::createFadeInAnimation(QWidget *widget, int duration)
{
    if (!widget) return nullptr;
    
    QPropertyAnimation *animation = new QPropertyAnimation(widget, "windowOpacity");
    animation->setDuration(duration);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    
    return animation;
}

QPropertyAnimation* StyleManager::createSlideInAnimation(QWidget *widget, 
                                                        const QRect &startGeometry,
                                                        const QRect &endGeometry,
                                                        int duration)
{
    if (!widget) return nullptr;
    
    QPropertyAnimation *animation = new QPropertyAnimation(widget, "geometry");
    animation->setDuration(duration);
    animation->setStartValue(startGeometry);
    animation->setEndValue(endGeometry);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    
    return animation;
}

QPropertyAnimation* StyleManager::createScaleAnimation(QWidget *widget, 
                                                      double startScale, 
                                                      double endScale,
                                                      int duration)
{
    if (!widget) return nullptr;
    
    // Use a property animation on a custom property
    widget->setProperty("scaleValue", startScale);
    
    QPropertyAnimation *animation = new QPropertyAnimation(widget, "scaleValue");
    animation->setDuration(duration);
    animation->setStartValue(startScale);
    animation->setEndValue(endScale);
    animation->setEasingCurve(QEasingCurve::OutBack);
    
    // Connect to update the actual scaling
    QObject::connect(animation, &QPropertyAnimation::valueChanged, 
                    [widget](const QVariant &value) {
        double scale = value.toDouble();
        widget->setStyleSheet(QString("transform: scale(%1);").arg(scale));
    });
    
    return animation;
}

void StyleManager::makeResponsive(QWidget *widget)
{
    if (!widget) return;
    
    widget->setProperty("responsive", true);
    
    // Enable automatic size adjustment
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void StyleManager::adjustForScreenSize(QWidget *widget, const QSize &screenSize)
{
    if (!widget) return;
    
    // Adjust font sizes based on screen size
    int baseFontSize = 14;
    if (screenSize.width() < 1024) {
        baseFontSize = 12;
    } else if (screenSize.width() > 1920) {
        baseFontSize = 16;
    }
    
    widget->setStyleSheet(widget->styleSheet() + 
                         QString("font-size: %1px;").arg(baseFontSize));
}

QString StyleManager::getPrimaryColor()
{
    return "#3D485A";
}

QString StyleManager::getAccentColor()
{
    return "#E3C6B0";
}

QString StyleManager::getSuccessColor()
{
    return "#4CAF50";
}

QString StyleManager::getWarningColor()
{
    return "#FFA726";
}

QString StyleManager::getErrorColor()
{
    return "#FF6B6B";
}

void StyleManager::addClass(QWidget *widget, const QString &className)
{
    if (!widget) return;
    
    QStringList classes = widget->property("cssClasses").toStringList();
    if (!classes.contains(className)) {
        classes.append(className);
        widget->setProperty("cssClasses", classes);
        
        // Apply class-specific styling
        if (className == "primary") {
            widget->setProperty("class", "primary");
        } else if (className == "success") {
            widget->setProperty("class", "success");
        } else if (className == "warning") {
            widget->setProperty("class", "warning");
        } else if (className == "error") {
            widget->setProperty("class", "error");
        }
        
        // Force style update
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
    }
}

void StyleManager::removeClass(QWidget *widget, const QString &className)
{
    if (!widget) return;
    
    QStringList classes = widget->property("cssClasses").toStringList();
    classes.removeAll(className);
    widget->setProperty("cssClasses", classes);
    
    // Force style update
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
}

bool StyleManager::hasClass(QWidget *widget, const QString &className)
{
    if (!widget) return false;
    
    QStringList classes = widget->property("cssClasses").toStringList();
    return classes.contains(className);
}

void StyleManager::setupColorPalette(QApplication *app)
{
    // Set application-wide palette
    QPalette palette = app->palette();
    
    // Primary colors
    palette.setColor(QPalette::Window, QColor("#FFFFFF"));
    palette.setColor(QPalette::WindowText, QColor("#3D485A"));
    palette.setColor(QPalette::Base, QColor("#FFFFFF"));
    palette.setColor(QPalette::AlternateBase, QColor("#F8F9FA"));
    palette.setColor(QPalette::Text, QColor("#495057"));
    palette.setColor(QPalette::Button, QColor("#E3C6B0"));
    palette.setColor(QPalette::ButtonText, QColor("#3D485A"));
    
    // Highlight colors
    palette.setColor(QPalette::Highlight, QColor("#3D485A"));
    palette.setColor(QPalette::HighlightedText, QColor("#E3C6B0"));
    
    app->setPalette(palette);
}

QString StyleManager::generateBoxShadow(int elevation)
{
    switch (elevation) {
    case 1:
        return "box-shadow: 0 2px 4px rgba(0,0,0,0.1);";
    case 2:
        return "box-shadow: 0 4px 8px rgba(0,0,0,0.15);";
    case 3:
        return "box-shadow: 0 6px 12px rgba(0,0,0,0.2);";
    case 4:
        return "box-shadow: 0 8px 16px rgba(0,0,0,0.25);";
    default:
        return "box-shadow: 0 10px 20px rgba(0,0,0,0.3);";
    }
}
