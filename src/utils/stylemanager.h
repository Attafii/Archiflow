#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H

#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QGraphicsEffect>
#include <QGraphicsDropShadowEffect>

/**
 * @brief The StyleManager class provides utilities for applying modern styling,
 * animations, and material design effects to Qt widgets
 */
class StyleManager : public QObject
{
    Q_OBJECT

public:
    explicit StyleManager(QObject *parent = nullptr);
    
    // Core styling methods
    static void applyArchiFlowTheme(QApplication *app);
    static void applyMaterialDesign(QWidget *widget);
    static void addElevation(QWidget *widget, int level = 1);
    static void addRippleEffect(QPushButton *button);
    
    // Animation methods
    static QPropertyAnimation* createFadeInAnimation(QWidget *widget, int duration = 300);
    static QPropertyAnimation* createSlideInAnimation(QWidget *widget, 
                                                     const QRect &startGeometry,
                                                     const QRect &endGeometry,
                                                     int duration = 400);
    static QPropertyAnimation* createScaleAnimation(QWidget *widget, 
                                                   double startScale = 0.8, 
                                                   double endScale = 1.0,
                                                   int duration = 300);
    
    // Responsive design
    static void makeResponsive(QWidget *widget);
    static void adjustForScreenSize(QWidget *widget, const QSize &screenSize);
    
    // Color utilities
    static QString getPrimaryColor();
    static QString getAccentColor();
    static QString getSuccessColor();
    static QString getWarningColor();
    static QString getErrorColor();
    
    // CSS classes
    static void addClass(QWidget *widget, const QString &className);
    static void removeClass(QWidget *widget, const QString &className);
    static bool hasClass(QWidget *widget, const QString &className);

private:
    static void setupColorPalette(QApplication *app);
    static QString generateBoxShadow(int elevation);
};

#endif // STYLEMANAGER_H
