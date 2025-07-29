#include "sidebar.h"
#include "utils/stylemanager.h"
#include "utils/animationmanager.h"
#include <QPainter>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QListWidgetItem>
#include <QDebug>
#include <QMenu>
#include <QActionGroup>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

Sidebar::Sidebar(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_logoContainer(nullptr)
    , m_logoLabel(nullptr)
    , m_logoTextLabel(nullptr)
    , m_navigationList(nullptr)
    , m_settingsContainer(nullptr)
    , m_settingsButton(nullptr)
    , m_minimized(false)
    , m_selectionAnimation(nullptr)
    , m_shadowEffect(nullptr)
{    setupUI();
    setupStyling();
    setupAnimations();
      setObjectName("sidebar");
    setFixedWidth(350);
    setMinimumHeight(600);
}

void Sidebar::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);    // Logo Container
    m_logoContainer = new QWidget(this);
    m_logoContainer->setObjectName("logoContainer");
    m_logoContainer->setFixedHeight(110);
    
    QVBoxLayout *logoLayout = new QVBoxLayout(m_logoContainer);
    logoLayout->setContentsMargins(30, 25, 30, 25);
    logoLayout->setSpacing(10);    m_logoLabel = new QLabel(this);
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setScaledContents(true);
    m_logoLabel->setMinimumSize(65, 65);
    m_logoLabel->setMaximumSize(65, 65);
    // Add a visible background and border for debugging
    m_logoLabel->setStyleSheet("background-color: #E3C6B0; border: 2px solid #D4B7A1; color: black;");
    m_logoLabel->setText("LOGO"); // Fallback text to see if the label is visible
    
    m_logoTextLabel = new QLabel("ArchiFlow", this);
    m_logoTextLabel->setObjectName("logoLabel");
    m_logoTextLabel->setAlignment(Qt::AlignCenter);
    
    logoLayout->addWidget(m_logoLabel, 0, Qt::AlignCenter);
    logoLayout->addWidget(m_logoTextLabel, 0, Qt::AlignCenter);
    
    // Navigation List
    m_navigationList = new QListWidget(this);
    m_navigationList->setObjectName("navigationList");
    m_navigationList->setFrameStyle(QFrame::NoFrame);
    m_navigationList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_navigationList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_navigationList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);    // Settings Container
    m_settingsContainer = new QWidget(this);
    m_settingsContainer->setObjectName("settingsContainer");
    m_settingsContainer->setFixedHeight(80);
    
    QHBoxLayout *settingsLayout = new QHBoxLayout(m_settingsContainer);
    settingsLayout->setContentsMargins(30, 15, 30, 15);
      m_settingsButton = new QPushButton("Settings", this);
    m_settingsButton->setObjectName("settingsButton");
    m_settingsButton->setCursor(Qt::PointingHandCursor);
    
    settingsLayout->addWidget(m_settingsButton);
    
    // Add to main layout
    m_mainLayout->addWidget(m_logoContainer);
    m_mainLayout->addWidget(m_navigationList, 1);
    m_mainLayout->addWidget(m_settingsContainer);
    
    // Connect signals
    connect(m_navigationList, &QListWidget::currentRowChanged,
            this, &Sidebar::onNavigationItemClicked);
    connect(m_settingsButton, &QPushButton::clicked,
            this, &Sidebar::onSettingsButtonClicked);
}

void Sidebar::setupStyling()
{
    // Apply shadow effect
    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setBlurRadius(10);
    m_shadowEffect->setOffset(2, 0);
    m_shadowEffect->setColor(QColor(0, 0, 0, 30));
    setGraphicsEffect(m_shadowEffect);
}

void Sidebar::setupAnimations()
{
    m_selectionAnimation = new QPropertyAnimation(this);
    m_selectionAnimation->setDuration(300);
    m_selectionAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void Sidebar::addNavigationItem(const QString &text, const QString &identifier, 
                               const QString &iconText)
{
    Q_UNUSED(iconText) // We're not using icons anymore
    
    QListWidgetItem *item = new QListWidgetItem(m_navigationList);
    item->setData(Qt::UserRole, identifier);
    item->setText(text); // Just use the text without icons
    
    // Store mapping for quick access
    m_identifierToRow[identifier] = m_navigationList->count() - 1;
    
    qDebug() << "Added navigation item:" << text << "with identifier:" << identifier;
}

void Sidebar::setCurrentItem(const QString &identifier)
{
    if (m_identifierToRow.contains(identifier)) {
        int row = m_identifierToRow[identifier];
        m_navigationList->setCurrentRow(row);
        m_currentIdentifier = identifier;
        animateItemSelection(row);
    }
}

QString Sidebar::currentItem() const
{
    return m_currentIdentifier;
}

void Sidebar::setLogo(const QPixmap &logo)
{
    qDebug() << "Sidebar::setLogo called.";
    if (!m_logoLabel) {
        qDebug() << "  Error: m_logoLabel is null!";
        return;
    }
    qDebug() << "  m_logoLabel is valid.";

    if (logo.isNull()) {
        qDebug() << "  Error: Passed logo QPixmap is null.";
        m_logoLabel->setText("Logo Error"); // Show error on label
        m_logoTextLabel->setVisible(true);
        m_logoTextLabel->setText("Pixmap Null");
        return;
    }
    qDebug() << "  Passed logo QPixmap is valid. Original size:" << logo.size();    QPixmap scaledLogo = logo.scaled(65, 65, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    qDebug() << "  Logo scaled to:" << scaledLogo.size();

    m_logoLabel->setText(""); // Clear any text
    m_logoLabel->setPixmap(scaledLogo);
    qDebug() << "  Pixmap set on m_logoLabel.";

    m_logoTextLabel->setVisible(false); // Hide text label when image is set
    qDebug() << "  m_logoTextLabel visibility set to false.";

    // Additional check for m_logoLabel visibility and properties
    qDebug() << "  m_logoLabel isVisible:" << m_logoLabel->isVisible();
    qDebug() << "  m_logoLabel geometry:" << m_logoLabel->geometry();
    qDebug() << "  m_logoLabel parentWidget:" << m_logoLabel->parentWidget();

}

void Sidebar::setLogoText(const QString &text)
{
    m_logoTextLabel->setText(text);
    m_logoTextLabel->setVisible(!text.isEmpty()); // Show only if text is not empty
}

void Sidebar::setMinimized(bool minimized)
{
    if (m_minimized == minimized) return;
    
    m_minimized = minimized;    // Animate width change
    QPropertyAnimation *widthAnimation = new QPropertyAnimation(this, "maximumWidth");
    widthAnimation->setDuration(300);
    widthAnimation->setStartValue(width());
    widthAnimation->setEndValue(minimized ? 70 : 350);
    widthAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    // Hide/show text labels
    m_logoTextLabel->setVisible(!minimized);
    
    widthAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

bool Sidebar::isMinimized() const
{
    return m_minimized;
}

void Sidebar::animateItemSelection(int index)
{
    if (index < 0 || index >= m_navigationList->count()) return;
    
    QListWidgetItem *item = m_navigationList->item(index);
    if (!item) return;
    
    // Visual feedback animation would be handled by the stylesheet
    // The selection animation is handled by CSS transitions
    updateItemStates();
}

void Sidebar::showSettingsMenu()
{
    QMenu *settingsMenu = new QMenu(this);
    settingsMenu->setAttribute(Qt::WA_DeleteOnClose);
    
    settingsMenu->addAction("🎨 Appearance", [this]() {
        // TODO: Open appearance settings
        qDebug() << "Appearance settings requested";
    });
    
    settingsMenu->addAction("🔧 Preferences", [this]() {
        // TODO: Open general preferences
        qDebug() << "General preferences requested";
    });
    
    settingsMenu->addSeparator();
    
    settingsMenu->addAction("ℹ About", [this]() {
        // TODO: Show about dialog
        qDebug() << "About dialog requested";
    });
    
    // Position menu above the button
    QPoint menuPosition = m_settingsButton->mapToGlobal(QPoint(0, -settingsMenu->sizeHint().height()));
    settingsMenu->popup(menuPosition);
}

void Sidebar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw gradient background
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, QColor("#2A3340"));
    gradient.setColorAt(1, QColor("#1F2937"));
    
    painter.fillRect(rect(), gradient);
    
    // Draw border
    painter.setPen(QPen(QColor("#E3C6B0"), 2));
    painter.drawLine(width() - 1, 0, width() - 1, height());
}

void Sidebar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateItemStates();
}

void Sidebar::onNavigationItemClicked(int row)
{
    if (row < 0 || row >= m_navigationList->count()) return;
    
    QListWidgetItem *item = m_navigationList->item(row);
    if (!item) return;
    
    QString identifier = item->data(Qt::UserRole).toString();
    m_currentIdentifier = identifier;
    
    animateItemSelection(row);
    
    qDebug() << "Navigation item clicked:" << item->text() << "(" << identifier << ")";
    emit navigationItemClicked(identifier);
}

void Sidebar::onSettingsButtonClicked()
{
    // Animate button press
    AnimationManager::animateButtonPress(m_settingsButton);
    
    // Show settings menu or emit signal
    showSettingsMenu();
    emit settingsClicked();
}

void Sidebar::updateItemStates()
{
    // Update visual states of navigation items
    for (int i = 0; i < m_navigationList->count(); ++i) {
        QListWidgetItem *item = m_navigationList->item(i);
        if (item) {
            // The visual updates are handled by CSS
            // This method can be used for additional state management
        }
    }
}

void Sidebar::animateHover(QListWidgetItem *item, bool entering)
{
    Q_UNUSED(item)
    Q_UNUSED(entering)
    
    // Hover animations are handled by CSS transitions
    // This method can be used for additional hover effects
}
