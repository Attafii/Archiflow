#include "animationmanager.h"
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <QDebug>

AnimationManager::AnimationManager(QObject *parent)
    : QObject(parent)
{
}

void AnimationManager::slidePageTransition(QWidget *fromPage, QWidget *toPage, 
                                          int duration, bool slideRight)
{
    if (!fromPage || !toPage) return;
    
    QWidget *parent = fromPage->parentWidget();
    if (!parent) return;
    
    // Setup positions
    QRect parentRect = parent->rect();
    QPoint fromStart = fromPage->pos();
    QPoint fromEnd = slideRight ? QPoint(-parentRect.width(), fromStart.y()) : 
                                  QPoint(parentRect.width(), fromStart.y());
    QPoint toStart = slideRight ? QPoint(parentRect.width(), fromStart.y()) : 
                                  QPoint(-parentRect.width(), fromStart.y());
    QPoint toEnd = fromStart;
    
    // Position the new page
    toPage->move(toStart);
    toPage->show();
    
    // Create animations
    QPropertyAnimation *fromAnim = createPositionAnimation(fromPage, fromStart, fromEnd, duration);
    QPropertyAnimation *toAnim = createPositionAnimation(toPage, toStart, toEnd, duration);
    
    // Create parallel group
    QParallelAnimationGroup *group = new QParallelAnimationGroup(parent);
    group->addAnimation(fromAnim);
    group->addAnimation(toAnim);
    
    // Hide the old page when done
    QObject::connect(group, &QAbstractAnimation::finished, [fromPage]() {
        fromPage->hide();
    });
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationManager::fadePageTransition(QWidget *fromPage, QWidget *toPage, int duration)
{
    if (!fromPage || !toPage) return;
    
    // Setup opacity effects
    QGraphicsOpacityEffect *fromEffect = getOrCreateOpacityEffect(fromPage);
    QGraphicsOpacityEffect *toEffect = getOrCreateOpacityEffect(toPage);
    
    fromEffect->setOpacity(1.0);
    toEffect->setOpacity(0.0);
    
    // Position the new page
    toPage->move(fromPage->pos());
    toPage->show();
    
    // Create fade animations
    QPropertyAnimation *fadeOut = new QPropertyAnimation(fromEffect, "opacity");
    fadeOut->setDuration(duration);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::OutCubic);
    
    QPropertyAnimation *fadeIn = new QPropertyAnimation(toEffect, "opacity");
    fadeIn->setDuration(duration);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::InCubic);
    
    // Create parallel group
    QParallelAnimationGroup *group = new QParallelAnimationGroup(fromPage->parent());
    group->addAnimation(fadeOut);
    group->addAnimation(fadeIn);
    
    // Hide the old page when done
    QObject::connect(group, &QAbstractAnimation::finished, [fromPage]() {
        fromPage->hide();
    });
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

QPropertyAnimation* AnimationManager::bounceIn(QWidget *widget, int duration)
{
    if (!widget) return nullptr;
    
    widget->setProperty("originalGeometry", widget->geometry());
    
    // Start small
    QRect startGeometry = widget->geometry();
    startGeometry.setSize(QSize(0, 0));
    startGeometry.moveCenter(widget->geometry().center());
    
    QPropertyAnimation *animation = new QPropertyAnimation(widget, "geometry");
    animation->setDuration(duration);
    animation->setStartValue(startGeometry);
    animation->setEndValue(widget->property("originalGeometry").toRect());
    animation->setEasingCurve(QEasingCurve::OutBack);
    
    return animation;
}

QPropertyAnimation* AnimationManager::slideInFromLeft(QWidget *widget, int duration)
{
    if (!widget) return nullptr;
    
    QRect endGeometry = widget->geometry();
    QRect startGeometry = endGeometry;
    startGeometry.moveLeft(-endGeometry.width());
    
    return createPositionAnimation(widget, startGeometry.topLeft(), 
                                 endGeometry.topLeft(), duration);
}

QPropertyAnimation* AnimationManager::slideInFromRight(QWidget *widget, int duration)
{
    if (!widget) return nullptr;
    
    QRect endGeometry = widget->geometry();
    QRect startGeometry = endGeometry;
    QWidget *parent = widget->parentWidget();
    if (parent) {
        startGeometry.moveLeft(parent->width());
    } else {
        startGeometry.moveLeft(endGeometry.right() + endGeometry.width());
    }
    
    return createPositionAnimation(widget, startGeometry.topLeft(), 
                                 endGeometry.topLeft(), duration);
}

QPropertyAnimation* AnimationManager::slideInFromTop(QWidget *widget, int duration)
{
    if (!widget) return nullptr;
    
    QRect endGeometry = widget->geometry();
    QRect startGeometry = endGeometry;
    startGeometry.moveTop(-endGeometry.height());
    
    return createPositionAnimation(widget, startGeometry.topLeft(), 
                                 endGeometry.topLeft(), duration);
}

QPropertyAnimation* AnimationManager::slideInFromBottom(QWidget *widget, int duration)
{
    if (!widget) return nullptr;
    
    QRect endGeometry = widget->geometry();
    QRect startGeometry = endGeometry;
    QWidget *parent = widget->parentWidget();
    if (parent) {
        startGeometry.moveTop(parent->height());
    } else {
        startGeometry.moveTop(endGeometry.bottom() + endGeometry.height());
    }
    
    return createPositionAnimation(widget, startGeometry.topLeft(), 
                                 endGeometry.topLeft(), duration);
}

void AnimationManager::animateButtonPress(QWidget *button)
{
    if (!button) return;
    
    QPropertyAnimation *scaleDown = new QPropertyAnimation(button, "geometry");
    scaleDown->setDuration(100);
    
    QRect currentGeometry = button->geometry();
    QRect scaledGeometry = currentGeometry;
    scaledGeometry.setSize(currentGeometry.size() * 0.95);
    scaledGeometry.moveCenter(currentGeometry.center());
    
    scaleDown->setStartValue(currentGeometry);
    scaleDown->setEndValue(scaledGeometry);
    scaleDown->setEasingCurve(QEasingCurve::OutCubic);
    
    QPropertyAnimation *scaleUp = new QPropertyAnimation(button, "geometry");
    scaleUp->setDuration(100);
    scaleUp->setStartValue(scaledGeometry);
    scaleUp->setEndValue(currentGeometry);
    scaleUp->setEasingCurve(QEasingCurve::OutCubic);
    
    QSequentialAnimationGroup *group = new QSequentialAnimationGroup(button);
    group->addAnimation(scaleDown);
    group->addAnimation(scaleUp);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationManager::animateButtonHover(QWidget *button, bool entering)
{
    if (!button) return;
    
    QGraphicsOpacityEffect *effect = getOrCreateOpacityEffect(button);
    
    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(200);
    animation->setStartValue(effect->opacity());
    animation->setEndValue(entering ? 0.8 : 1.0);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

QPropertyAnimation* AnimationManager::createPulseAnimation(QWidget *widget)
{
    if (!widget) return nullptr;
    
    QGraphicsOpacityEffect *effect = getOrCreateOpacityEffect(widget);
    
    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(1000);
    animation->setStartValue(1.0);
    animation->setEndValue(0.3);
    animation->setEasingCurve(QEasingCurve::InOutCubic);
    animation->setLoopCount(-1); // Infinite
    
    // Make it alternate (pulse in and out)
    animation->setDirection(QAbstractAnimation::Forward);
    
    QTimer *reverseTimer = new QTimer(widget);
    QObject::connect(animation, &QAbstractAnimation::finished, [animation, reverseTimer]() {
        if (animation->direction() == QAbstractAnimation::Forward) {
            animation->setDirection(QAbstractAnimation::Backward);
        } else {
            animation->setDirection(QAbstractAnimation::Forward);
        }
    });
    
    return animation;
}

QPropertyAnimation* AnimationManager::createRotationAnimation(QWidget *widget)
{
    if (!widget) return nullptr;
    
    // Note: Qt doesn't have a built-in rotation property for widgets
    // This would require a custom property or transform
    widget->setProperty("rotation", 0);
    
    QPropertyAnimation *animation = new QPropertyAnimation(widget, "rotation");
    animation->setDuration(2000);
    animation->setStartValue(0);
    animation->setEndValue(360);
    animation->setLoopCount(-1);
    animation->setEasingCurve(QEasingCurve::Linear);
    
    return animation;
}

void AnimationManager::animateListItemInsertion(QWidget *item, int duration)
{
    if (!item) return;
    
    // Slide in from right with fade
    QGraphicsOpacityEffect *effect = getOrCreateOpacityEffect(item);
    effect->setOpacity(0.0);
    
    QRect endGeometry = item->geometry();
    QRect startGeometry = endGeometry;
    startGeometry.moveLeft(endGeometry.right() + 50);
    item->setGeometry(startGeometry);
    
    // Fade in
    QPropertyAnimation *fadeAnim = new QPropertyAnimation(effect, "opacity");
    fadeAnim->setDuration(duration);
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setEasingCurve(QEasingCurve::OutCubic);
    
    // Slide in
    QPropertyAnimation *slideAnim = createPositionAnimation(item, 
                                                           startGeometry.topLeft(),
                                                           endGeometry.topLeft(),
                                                           duration);
    
    QParallelAnimationGroup *group = new QParallelAnimationGroup(item);
    group->addAnimation(fadeAnim);
    group->addAnimation(slideAnim);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationManager::animateListItemRemoval(QWidget *item, int duration)
{
    if (!item) return;
    
    QGraphicsOpacityEffect *effect = getOrCreateOpacityEffect(item);
    
    QPropertyAnimation *fadeAnim = new QPropertyAnimation(effect, "opacity");
    fadeAnim->setDuration(duration);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);
    fadeAnim->setEasingCurve(QEasingCurve::InCubic);
    
    QObject::connect(fadeAnim, &QAbstractAnimation::finished, [item]() {
        item->deleteLater();
    });
    
    fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationManager::animateModalShow(QWidget *modal, int duration)
{
    if (!modal) return;
    
    QGraphicsOpacityEffect *effect = getOrCreateOpacityEffect(modal);
    effect->setOpacity(0.0);
    
    // Scale effect
    modal->setProperty("originalGeometry", modal->geometry());
    QRect startGeometry = modal->geometry();
    startGeometry.setSize(startGeometry.size() * 0.8);
    startGeometry.moveCenter(modal->geometry().center());
    modal->setGeometry(startGeometry);
    
    // Fade in
    QPropertyAnimation *fadeAnim = new QPropertyAnimation(effect, "opacity");
    fadeAnim->setDuration(duration);
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setEasingCurve(QEasingCurve::OutCubic);
    
    // Scale up
    QPropertyAnimation *scaleAnim = new QPropertyAnimation(modal, "geometry");
    scaleAnim->setDuration(duration);
    scaleAnim->setStartValue(startGeometry);
    scaleAnim->setEndValue(modal->property("originalGeometry").toRect());
    scaleAnim->setEasingCurve(QEasingCurve::OutBack);
    
    QParallelAnimationGroup *group = new QParallelAnimationGroup(modal);
    group->addAnimation(fadeAnim);
    group->addAnimation(scaleAnim);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationManager::animateModalHide(QWidget *modal, int duration)
{
    if (!modal) return;
    
    QGraphicsOpacityEffect *effect = getOrCreateOpacityEffect(modal);
    
    QPropertyAnimation *fadeAnim = new QPropertyAnimation(effect, "opacity");
    fadeAnim->setDuration(duration);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);
    fadeAnim->setEasingCurve(QEasingCurve::InCubic);
    
    QObject::connect(fadeAnim, &QAbstractAnimation::finished, [modal]() {
        modal->hide();
    });
    
    fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationManager::showNotification(QWidget *notification, QWidget *parent, int duration)
{
    if (!notification || !parent) return;
    
    // Position at top-right of parent
    QPoint position(parent->width() - notification->width() - 20, 20);
    QPoint startPosition(parent->width(), 20);
    
    notification->move(startPosition);
    notification->show();
    
    // Slide in
    QPropertyAnimation *slideIn = createPositionAnimation(notification, 
                                                         startPosition, 
                                                         position, 
                                                         300);
    slideIn->start(QAbstractAnimation::DeleteWhenStopped);
    
    // Auto-hide after duration
    QTimer::singleShot(duration, [notification, parent]() {
        QPoint hidePosition(parent->width(), notification->y());
        QPropertyAnimation *slideOut = createPositionAnimation(notification,
                                                             notification->pos(),
                                                             hidePosition,
                                                             300);
        QObject::connect(slideOut, &QAbstractAnimation::finished, [notification]() {
            notification->hide();
        });
        slideOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

QSequentialAnimationGroup* AnimationManager::createSequentialAnimation(
    const QList<QPropertyAnimation*> &animations)
{
    QSequentialAnimationGroup *group = new QSequentialAnimationGroup();
    for (QPropertyAnimation *anim : animations) {
        group->addAnimation(anim);
    }
    return group;
}

QParallelAnimationGroup* AnimationManager::createParallelAnimation(
    const QList<QPropertyAnimation*> &animations)
{
    QParallelAnimationGroup *group = new QParallelAnimationGroup();
    for (QPropertyAnimation *anim : animations) {
        group->addAnimation(anim);
    }
    return group;
}

void AnimationManager::playAnimation(QPropertyAnimation *animation)
{
    if (!animation) return;
    
    m_activeAnimations.append(animation);
    
    QObject::connect(animation, &QAbstractAnimation::finished, [this, animation]() {
        m_activeAnimations.removeOne(animation);
        emit animationFinished(animation->objectName());
    });
    
    animation->start();
}

void AnimationManager::stopAllAnimations()
{
    for (QPropertyAnimation *animation : m_activeAnimations) {
        animation->stop();
    }
    m_activeAnimations.clear();
}

QPropertyAnimation* AnimationManager::createPositionAnimation(QWidget *widget, 
                                                             const QPoint &startPos,
                                                             const QPoint &endPos,
                                                             int duration)
{
    if (!widget) return nullptr;
    
    QPropertyAnimation *animation = new QPropertyAnimation(widget, "pos");
    animation->setDuration(duration);
    animation->setStartValue(startPos);
    animation->setEndValue(endPos);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    
    return animation;
}

QGraphicsOpacityEffect* AnimationManager::getOrCreateOpacityEffect(QWidget *widget)
{
    if (!widget) return nullptr;
    
    QGraphicsOpacityEffect *effect = 
        qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    
    if (!effect) {
        effect = new QGraphicsOpacityEffect(widget);
        effect->setOpacity(1.0);
        widget->setGraphicsEffect(effect);
    }
    
    return effect;
}
