#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

/**
 * @brief The Sidebar class provides a modern navigation sidebar for ArchiFlow
 * 
 * Features:
 * - Logo display area at the top
 * - Navigation items with smooth animations
 * - Settings button at the bottom
 * - Material design styling
 * - Responsive behavior
 */
class Sidebar : public QWidget
{
    Q_OBJECT

public:
    explicit Sidebar(QWidget *parent = nullptr);
      // Navigation management
    void addNavigationItem(const QString &text, const QString &identifier, 
                          const QString &iconText = QString());
    void setCurrentItem(const QString &identifier);
    QString currentItem() const;
    
    // Logo management
    void setLogo(const QPixmap &logo);
    void setLogoText(const QString &text);
    
    // Styling
    void setMinimized(bool minimized);
    bool isMinimized() const;

public slots:
    void animateItemSelection(int index);
    void showSettingsMenu();

signals:
    void navigationItemClicked(const QString &identifier);
    void settingsClicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onNavigationItemClicked(int row);
    void onSettingsButtonClicked();

private:
    void setupUI();
    void setupStyling();
    void setupAnimations();
    void updateItemStates();
    void animateHover(QListWidgetItem *item, bool entering);
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    
    // Logo section
    QWidget *m_logoContainer;
    QLabel *m_logoLabel;
    QLabel *m_logoTextLabel;
    
    // Navigation section
    QListWidget *m_navigationList;
    
    // Settings section
    QWidget *m_settingsContainer;
    QPushButton *m_settingsButton;
    
    // State
    QString m_currentIdentifier;
    bool m_minimized;
    QMap<QString, int> m_identifierToRow;
    
    // Animation
    QPropertyAnimation *m_selectionAnimation;
    QGraphicsDropShadowEffect *m_shadowEffect;
};

#endif // SIDEBAR_H
