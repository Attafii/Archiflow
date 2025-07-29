#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include <QObject>
#include <QWidget>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QGraphicsOpacityEffect>

/**
 * @brief The AnimationManager class handles all animations and transitions
 * for the ArchiFlow application, providing smooth and modern UI interactions
 */
class AnimationManager : public QObject
{
    Q_OBJECT

public:
    explicit AnimationManager(QObject *parent = nullptr);
    
    // Page transitions
    static void slidePageTransition(QWidget *fromPage, QWidget *toPage, 
                                   int duration = 400, bool slideRight = true);
    static void fadePageTransition(QWidget *fromPage, QWidget *toPage, 
                                  int duration = 300);
    
    // Widget animations
    static QPropertyAnimation* bounceIn(QWidget *widget, int duration = 600);
    static QPropertyAnimation* slideInFromLeft(QWidget *widget, int duration = 400);
    static QPropertyAnimation* slideInFromRight(QWidget *widget, int duration = 400);
    static QPropertyAnimation* slideInFromTop(QWidget *widget, int duration = 400);
    static QPropertyAnimation* slideInFromBottom(QWidget *widget, int duration = 400);
    
    // Button effects
    static void animateButtonPress(QWidget *button);
    static void animateButtonHover(QWidget *button, bool entering);
    
    // Loading animations
    static QPropertyAnimation* createPulseAnimation(QWidget *widget);
    static QPropertyAnimation* createRotationAnimation(QWidget *widget);
    
    // List/Table animations
    static void animateListItemInsertion(QWidget *item, int duration = 300);
    static void animateListItemRemoval(QWidget *item, int duration = 300);
    
    // Modal/Dialog animations
    static void animateModalShow(QWidget *modal, int duration = 350);
    static void animateModalHide(QWidget *modal, int duration = 250);
    
    // Notification animations
    static void showNotification(QWidget *notification, QWidget *parent, 
                               int duration = 4000);
    
    // Chain animations
    static QSequentialAnimationGroup* createSequentialAnimation(
        const QList<QPropertyAnimation*> &animations);
    static QParallelAnimationGroup* createParallelAnimation(
        const QList<QPropertyAnimation*> &animations);

public slots:
    void playAnimation(QPropertyAnimation *animation);
    void stopAllAnimations();

signals:
    void animationFinished(const QString &animationName);

private:
    static QPropertyAnimation* createPositionAnimation(QWidget *widget, 
                                                      const QPoint &startPos,
                                                      const QPoint &endPos,
                                                      int duration);
    static QGraphicsOpacityEffect* getOrCreateOpacityEffect(QWidget *widget);
    
    QList<QPropertyAnimation*> m_activeAnimations;
};

#endif // ANIMATIONMANAGER_H
