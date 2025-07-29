#include "aiassistantdialog.h"
#include "utils/documentprocessor.h"
#include "database/databaseservice.h"
#include <QApplication>
#include <QScreen>
#include <QGraphicsDropShadowEffect>
#include <QTextDocument>
#include <QTextCursor>
#include <QScrollBar>
#include <QDateTime>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QKeyEvent>
#include <QShortcut>
#include <QClipboard>
#include <QToolTip>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QEasingCurve>
#include <QRegularExpression>

// ChatBubble Implementation
ChatBubble::ChatBubble(Type type, const QString &message, QWidget *parent)
    : QFrame(parent)
    , m_type(type)
    , m_message(message)
    , m_typingDots(0)
{
    setupUI();
    setupAnimations();
    setMessage(message);
}

void ChatBubble::setupUI()
{
    setFrameStyle(QFrame::NoFrame);
    setContentsMargins(10, 8, 10, 8);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Avatar
    m_avatarLabel = new QLabel();
    m_avatarLabel->setFixedSize(32, 32);
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    m_avatarLabel->setStyleSheet(
        "border-radius: 16px; "
        "font-weight: bold; "
        "font-size: 14px;"
    );
    
    // Message content
    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->setSpacing(4);
    
    m_messageLabel = new QLabel();
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    m_messageLabel->setOpenExternalLinks(true);
    m_messageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    m_timestampLabel = new QLabel();
    m_timestampLabel->setStyleSheet("color: #888; font-size: 11px;");
    m_timestampLabel->setText(QDateTime::currentDateTime().toString("hh:mm"));
    
    contentLayout->addWidget(m_messageLabel);
    contentLayout->addWidget(m_timestampLabel);
    
    // Layout based on type
    if (m_type == User) {
        mainLayout->addStretch();
        mainLayout->addLayout(contentLayout);
        mainLayout->addWidget(m_avatarLabel);
        
        m_avatarLabel->setText("U");
        m_avatarLabel->setStyleSheet(m_avatarLabel->styleSheet() + "background-color: #E3C6B0; color: #3D485A;");
        
        setStyleSheet(
            "ChatBubble { "
            "background-color: #E3C6B0; "
            "border-radius: 18px 18px 4px 18px; "
            "margin-left: 50px; "
            "margin-right: 10px; "
            "}"
        );
        m_messageLabel->setStyleSheet("color: #3D485A; font-size: 14px; padding: 8px;");
        
    } else {
        mainLayout->addWidget(m_avatarLabel);
        mainLayout->addLayout(contentLayout);
        mainLayout->addStretch();
        
        m_avatarLabel->setText("AI");
        m_avatarLabel->setStyleSheet(m_avatarLabel->styleSheet() + "background-color: #3D485A; color: #E3C6B0;");
        
        setStyleSheet(
            "ChatBubble { "
            "background-color: #2A3340; "
            "border-radius: 18px 18px 18px 4px; "
            "margin-left: 10px; "
            "margin-right: 50px; "
            "border: 1px solid #3D485A; "
            "}"
        );
        m_messageLabel->setStyleSheet("color: #E3C6B0; font-size: 14px; padding: 8px;");
    }
    
    // Add drop shadow
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(8);
    shadow->setOffset(0, 2);
    shadow->setColor(QColor(0, 0, 0, 30));
    setGraphicsEffect(shadow);
}

void ChatBubble::setupAnimations()
{
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(0.0);
    
    m_fadeInAnimation = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_fadeInAnimation->setDuration(300);
    m_fadeInAnimation->setEasingCurve(QEasingCurve::OutQuart);
    
    m_slideInAnimation = new QPropertyAnimation(this, "geometry", this);
    m_slideInAnimation->setDuration(300);
    m_slideInAnimation->setEasingCurve(QEasingCurve::OutQuart);
    
    m_typingTimer = new QTimer(this);
    connect(m_typingTimer, &QTimer::timeout, [this]() {
        if (m_type == Assistant) {
            m_typingDots = (m_typingDots + 1) % 4;
            QString dots = QString(".").repeated(m_typingDots);
            m_messageLabel->setText("Thinking" + dots);
        }
    });
}

void ChatBubble::setMessage(const QString &message)
{
    m_message = message;
    m_messageLabel->setText(message);
}

void ChatBubble::setTimestamp(const QDateTime &timestamp)
{
    m_timestampLabel->setText(timestamp.toString("hh:mm"));
}

void ChatBubble::animateIn()
{
    setGraphicsEffect(m_opacityEffect);
    
    // Start from 30px below
    QRect finalGeometry = geometry();
    QRect startGeometry = finalGeometry;
    startGeometry.moveTop(finalGeometry.top() + 30);
    setGeometry(startGeometry);
    
    m_slideInAnimation->setStartValue(startGeometry);
    m_slideInAnimation->setEndValue(finalGeometry);
    
    m_fadeInAnimation->setStartValue(0.0);
    m_fadeInAnimation->setEndValue(1.0);
    
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(m_fadeInAnimation);
    group->addAnimation(m_slideInAnimation);
    
    connect(group, &QAbstractAnimation::finished, [this]() {
        setGraphicsEffect(nullptr);
    });
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void ChatBubble::startTypingAnimation()
{
    if (m_type == Assistant) {
        m_typingTimer->start(500);
    }
}

void ChatBubble::stopTypingAnimation()
{
    m_typingTimer->stop();
}

// TypingIndicator Implementation
TypingIndicator::TypingIndicator(QWidget *parent)
    : QWidget(parent)
    , m_currentDot(0)
{
    setupUI();
    setVisible(false);
}

void TypingIndicator::setupUI()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(20, 10, 20, 10);
    m_layout->setSpacing(4);
    
    QLabel *aiLabel = new QLabel("AI");
    aiLabel->setStyleSheet(
        "background-color: #3D485A; "
        "color: #E3C6B0; "
        "border-radius: 16px; "
        "font-weight: bold; "
        "font-size: 14px; "
        "min-width: 32px; "
        "max-width: 32px; "
        "min-height: 32px; "
        "max-height: 32px;"
    );
    aiLabel->setAlignment(Qt::AlignCenter);
    
    m_layout->addWidget(aiLabel);
    
    QLabel *typingLabel = new QLabel("is typing");
    typingLabel->setStyleSheet("color: #888; font-size: 12px; margin-left: 8px;");
    m_layout->addWidget(typingLabel);
    
    // Animated dots
    m_dot1 = new QLabel("•");
    m_dot2 = new QLabel("•");
    m_dot3 = new QLabel("•");
    
    QString dotStyle = "color: #3D485A; font-size: 16px; font-weight: bold;";
    m_dot1->setStyleSheet(dotStyle);
    m_dot2->setStyleSheet(dotStyle);
    m_dot3->setStyleSheet(dotStyle);
    
    m_layout->addWidget(m_dot1);
    m_layout->addWidget(m_dot2);
    m_layout->addWidget(m_dot3);
    m_layout->addStretch();
    
    // Setup animations
    m_animation1 = new QPropertyAnimation(m_dot1, "geometry", this);
    m_animation2 = new QPropertyAnimation(m_dot2, "geometry", this);
    m_animation3 = new QPropertyAnimation(m_dot3, "geometry", this);
    
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &TypingIndicator::updateAnimation);
}

void TypingIndicator::show()
{
    QWidget::show();
    m_animationTimer->start(400);
    updateAnimation();
}

void TypingIndicator::hide()
{
    m_animationTimer->stop();
    QWidget::hide();
}

void TypingIndicator::updateAnimation()
{
    QString activeStyle = "color: #E3C6B0; font-size: 16px; font-weight: bold;";
    QString inactiveStyle = "color: #555; font-size: 16px; font-weight: bold;";
    
    m_dot1->setStyleSheet(m_currentDot == 0 ? activeStyle : inactiveStyle);
    m_dot2->setStyleSheet(m_currentDot == 1 ? activeStyle : inactiveStyle);
    m_dot3->setStyleSheet(m_currentDot == 2 ? activeStyle : inactiveStyle);
    
    m_currentDot = (m_currentDot + 1) % 3;
}

// AIAssistantDialog Implementation
AIAssistantDialog::AIAssistantDialog(QWidget *parent)
    : QDialog(parent)
    , m_groqClient(nullptr)
    , m_databaseService(nullptr)
    , m_autoScroll(true)
    , m_showTimestamps(true)
    , m_fontFamily("Poppins")
    , m_fontSize(14)
{    setWindowTitle("AI Assistant - ArchiFlow Materials");
    setModal(false);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint);
    
    // Prevent the dialog from auto-deleting when closed
    setAttribute(Qt::WA_DeleteOnClose, false);
    
    // Enable drag and drop for document upload
    setAcceptDrops(true);
    
    // Set initial size and position
    resize(800, 600);
    
    setupUI();
    setupAnimations();
    setupConnections();
    setupQuickActions();
    applyArchiFlowTheme();
    loadSettings();
    
    // Initialize with welcome message
    addMessage(ChatBubble::Assistant, 
               "👋 Hello! I'm your AI assistant for materials management. I can help you with:\n"
               "• Material specifications and properties\n"
               "• Cost estimation and analysis\n"
               "• Supplier recommendations\n"
               "• Construction project planning\n"
               "• Inventory management tips\n\n"
               "How can I assist you today?");
}

AIAssistantDialog::~AIAssistantDialog()
{
    saveSettings();
}

void AIAssistantDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Header
    m_headerFrame = new QFrame();
    m_headerFrame->setObjectName("headerFrame");
    m_headerFrame->setFixedHeight(60);
    
    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerFrame);
    headerLayout->setContentsMargins(20, 10, 20, 10);
    
    m_titleLabel = new QLabel("AI Assistant");
    m_titleLabel->setObjectName("titleLabel");
    
    m_connectionIndicator = new QLabel();
    m_connectionIndicator->setFixedSize(12, 12);
    m_connectionIndicator->setStyleSheet("border-radius: 6px; background-color: #f44336;");
    
    QLabel *connectionLabel = new QLabel("Offline");
    connectionLabel->setObjectName("connectionLabel");
    
    m_settingsButton = new QPushButton("⚙");
    m_settingsButton->setObjectName("headerButton");
    m_settingsButton->setFixedSize(36, 36);
    
    m_closeButton = new QPushButton("✕");
    m_closeButton->setObjectName("headerButton");
    m_closeButton->setFixedSize(36, 36);
    
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_connectionIndicator);
    headerLayout->addWidget(connectionLabel);
    headerLayout->addSpacing(20);
    headerLayout->addWidget(m_settingsButton);
    headerLayout->addWidget(m_closeButton);
    
    // Main content area
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);
    
    // Chat area
    QWidget *chatWidget = new QWidget();
    QVBoxLayout *chatMainLayout = new QVBoxLayout(chatWidget);
    chatMainLayout->setContentsMargins(0, 0, 0, 0);
    chatMainLayout->setSpacing(0);
    
    // Chat scroll area
    m_chatScrollArea = new QScrollArea();
    m_chatScrollArea->setObjectName("chatScrollArea");
    m_chatScrollArea->setWidgetResizable(true);
    m_chatScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_chatScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_chatWidget = new QWidget();
    m_chatLayout = new QVBoxLayout(m_chatWidget);
    m_chatLayout->setSpacing(12);
    m_chatLayout->setContentsMargins(20, 20, 20, 20);
    m_chatLayout->addStretch();
    
    m_chatScrollArea->setWidget(m_chatWidget);
    
    // Typing indicator
    m_typingIndicator = new TypingIndicator();
    m_chatLayout->insertWidget(m_chatLayout->count() - 1, m_typingIndicator);
    
    chatMainLayout->addWidget(m_chatScrollArea);
    
    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setObjectName("progressBar");
    m_progressBar->setVisible(false);
    m_progressBar->setRange(0, 0); // Indeterminate
    chatMainLayout->addWidget(m_progressBar);
    
    // Input frame
    m_inputFrame = new QFrame();
    m_inputFrame->setObjectName("inputFrame");
    m_inputFrame->setFixedHeight(80);
    
    QHBoxLayout *inputLayout = new QHBoxLayout(m_inputFrame);
    inputLayout->setContentsMargins(20, 15, 20, 15);
    inputLayout->setSpacing(10);
      m_attachButton = new QPushButton("📎");
    m_attachButton->setObjectName("inputButton");
    m_attachButton->setFixedSize(40, 40);
    m_attachButton->setToolTip("Attach document for analysis (PDF, CSV, Excel)");
      m_messageInput = new QLineEdit();
    m_messageInput->setObjectName("messageInput");
    m_messageInput->setPlaceholderText("Type your message or drag & drop documents here...");
    
    m_voiceButton = new QPushButton("🎤");
    m_voiceButton->setObjectName("inputButton");
    m_voiceButton->setFixedSize(40, 40);
    
    m_sendButton = new QPushButton("Send");
    m_sendButton->setObjectName("sendButton");
    m_sendButton->setFixedSize(80, 40);
    m_sendButton->setEnabled(false);
    
    inputLayout->addWidget(m_attachButton);
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_voiceButton);
    inputLayout->addWidget(m_sendButton);
    
    chatMainLayout->addWidget(m_inputFrame);
    
    // Sidebar
    m_sidebarFrame = new QFrame();
    m_sidebarFrame->setObjectName("sidebarFrame");
    m_sidebarFrame->setFixedWidth(250);
    
    m_sidebarLayout = new QVBoxLayout(m_sidebarFrame);
    m_sidebarLayout->setContentsMargins(15, 20, 15, 20);
    m_sidebarLayout->setSpacing(15);
    
    m_quickActionsLabel = new QLabel("Quick Actions");
    m_quickActionsLabel->setObjectName("sidebarTitle");
    
    m_quickActionsList = new QListWidget();
    m_quickActionsList->setObjectName("quickActionsList");
    
    QPushButton *clearChatBtn = new QPushButton("Clear Chat");
    QPushButton *exportChatBtn = new QPushButton("Export Chat");
    
    clearChatBtn->setObjectName("sidebarButton");
    exportChatBtn->setObjectName("sidebarButton");
    
    m_sidebarLayout->addWidget(m_quickActionsLabel);
    m_sidebarLayout->addWidget(m_quickActionsList);
    m_sidebarLayout->addStretch();
    m_sidebarLayout->addWidget(clearChatBtn);
    m_sidebarLayout->addWidget(exportChatBtn);
    
    // Add to splitter
    mainSplitter->addWidget(chatWidget);
    mainSplitter->addWidget(m_sidebarFrame);
    mainSplitter->setSizes({600, 200});
    
    // Add to main layout
    m_mainLayout->addWidget(m_headerFrame);
    m_mainLayout->addWidget(mainSplitter);
    
    // Connect sidebar buttons
    connect(clearChatBtn, &QPushButton::clicked, this, &AIAssistantDialog::clearChat);
    connect(exportChatBtn, &QPushButton::clicked, this, &AIAssistantDialog::exportChat);
}

void AIAssistantDialog::setupAnimations()
{
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(0.0);
    
    m_fadeInAnimation = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_fadeInAnimation->setDuration(400);
    m_fadeInAnimation->setEasingCurve(QEasingCurve::OutQuart);
    
    m_scaleAnimation = new QPropertyAnimation(this, "geometry", this);
    m_scaleAnimation->setDuration(400);
    m_scaleAnimation->setEasingCurve(QEasingCurve::OutBack);
}

void AIAssistantDialog::setupConnections()
{
    // Input connections
    connect(m_messageInput, &QLineEdit::textChanged, [this](const QString &text) {
        m_sendButton->setEnabled(!text.trimmed().isEmpty());
    });
      connect(m_messageInput, &QLineEdit::returnPressed, this, &AIAssistantDialog::sendMessage);
    connect(m_sendButton, &QPushButton::clicked, this, &AIAssistantDialog::sendMessage);
    connect(m_attachButton, &QPushButton::clicked, this, &AIAssistantDialog::attachDocument);
    
    // Header connections
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::close);
    connect(m_settingsButton, &QPushButton::clicked, this, &AIAssistantDialog::showSettings);
    
    // Quick actions
    connect(m_quickActionsList, &QListWidget::itemDoubleClicked, this, &AIAssistantDialog::onQuickActionClicked);
    
    // Keyboard shortcuts
    QShortcut *sendShortcut = new QShortcut(QKeySequence("Ctrl+Return"), this);
    connect(sendShortcut, &QShortcut::activated, this, &AIAssistantDialog::sendMessage);
    
    QShortcut *clearShortcut = new QShortcut(QKeySequence("Ctrl+L"), this);
    connect(clearShortcut, &QShortcut::activated, this, &AIAssistantDialog::clearChat);
}

void AIAssistantDialog::setupQuickActions()
{
    m_quickActions = {
        "📊 Analyze material costs",
        "🔍 Compare material options",
        "📝 Create material specification",
        "💰 Calculate project budget",
        "🏭 Find suppliers near me",
        "📈 Market price trends",
        "⚖️ Material quantity calculator",
        "🛠️ Installation requirements",
        "🌱 Sustainable alternatives",
        "📋 Quality standards",
        "📄 Upload & analyze document"
    };
    
    for (const QString &action : m_quickActions) {
        m_quickActionsList->addItem(action);
    }
}

void AIAssistantDialog::applyArchiFlowTheme()
{
    setStyleSheet(
        "AIAssistantDialog {"
        "    background-color: #2A3340;"
        "    color: #E3C6B0;"
        "}"
        
        "#headerFrame {"
        "    background-color: #3D485A;"
        "    border-bottom: 2px solid #E3C6B0;"
        "}"
        
        "#titleLabel {"
        "    font-family: 'Poppins', sans-serif;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    color: #E3C6B0;"
        "}"
        
        "#connectionLabel {"
        "    font-size: 12px;"
        "    color: #E3C6B0;"
        "}"
        
        "#headerButton {"
        "    background-color: transparent;"
        "    border: 1px solid #E3C6B0;"
        "    border-radius: 18px;"
        "    color: #E3C6B0;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "}"
        
        "#headerButton:hover {"
        "    background-color: #E3C6B0;"
        "    color: #3D485A;"
        "}"
        
        "#chatScrollArea {"
        "    background-color: #2A3340;"
        "    border: none;"
        "}"
        
        "#inputFrame {"
        "    background-color: #3D485A;"
        "    border-top: 1px solid #E3C6B0;"
        "}"
        
        "#messageInput {"
        "    background-color: #2A3340;"
        "    border: 2px solid #E3C6B0;"
        "    border-radius: 20px;"
        "    padding: 10px 15px;"
        "    font-size: 14px;"
        "    color: #E3C6B0;"
        "    font-family: 'Poppins', sans-serif;"
        "}"
        
        "#messageInput:focus {"
        "    border-color: #B8860B;"
        "    outline: none;"
        "}"
        
        "#sendButton {"
        "    background-color: #E3C6B0;"
        "    border: none;"
        "    border-radius: 20px;"
        "    color: #3D485A;"
        "    font-weight: bold;"
        "    font-family: 'Poppins', sans-serif;"
        "}"
        
        "#sendButton:hover {"
        "    background-color: #B8860B;"
        "}"
        
        "#sendButton:disabled {"
        "    background-color: #555;"
        "    color: #888;"
        "}"
        
        "#inputButton {"
        "    background-color: transparent;"
        "    border: 1px solid #E3C6B0;"
        "    border-radius: 20px;"
        "    font-size: 16px;"
        "}"
        
        "#inputButton:hover {"
        "    background-color: #E3C6B0;"
        "    color: #3D485A;"
        "}"
        
        "#sidebarFrame {"
        "    background-color: #3D485A;"
        "    border-left: 1px solid #E3C6B0;"
        "}"
        
        "#sidebarTitle {"
        "    font-weight: bold;"
        "    font-size: 16px;"
        "    color: #E3C6B0;"
        "    font-family: 'Poppins', sans-serif;"
        "}"
        
        "#quickActionsList {"
        "    background-color: #2A3340;"
        "    border: 1px solid #E3C6B0;"
        "    border-radius: 8px;"
        "    color: #E3C6B0;"
        "    selection-background-color: #E3C6B0;"
        "    selection-color: #3D485A;"
        "    font-family: 'Poppins', sans-serif;"
        "}"
        
        "#quickActionsList::item {"
        "    padding: 8px;"
        "    border-bottom: 1px solid #3D485A;"
        "}"
        
        "#quickActionsList::item:hover {"
        "    background-color: #3D485A;"
        "}"
        
        "#sidebarButton {"
        "    background-color: #2A3340;"
        "    border: 1px solid #E3C6B0;"
        "    border-radius: 8px;"
        "    color: #E3C6B0;"
        "    padding: 10px;"
        "    font-family: 'Poppins', sans-serif;"
        "}"
        
        "#sidebarButton:hover {"
        "    background-color: #E3C6B0;"
        "    color: #3D485A;"
        "}"
        
        "#progressBar {"
        "    background-color: #2A3340;"
        "    border: 1px solid #E3C6B0;"
        "    border-radius: 4px;"
        "}"
        
        "#progressBar::chunk {"
        "    background-color: #E3C6B0;"
        "    border-radius: 3px;"
        "}"
    );
}

void AIAssistantDialog::setGroqClient(GroqClient *client)
{
    if (m_groqClient) {
        disconnect(m_groqClient, nullptr, this, nullptr);
    }
    
    m_groqClient = client;
    
    if (m_groqClient) {
        connect(m_groqClient, &GroqClient::messageReceived, this, &AIAssistantDialog::onMessageReceived);
        connect(m_groqClient, &GroqClient::errorOccurred, this, &AIAssistantDialog::onErrorOccurred);
        connect(m_groqClient, &GroqClient::connectionStatusChanged, this, &AIAssistantDialog::onConnectionStatusChanged);
        connect(m_groqClient, &GroqClient::requestStarted, this, &AIAssistantDialog::onRequestStarted);
        connect(m_groqClient, &GroqClient::requestFinished, this, &AIAssistantDialog::onRequestFinished);
        connect(m_groqClient, &GroqClient::typingStarted, this, &AIAssistantDialog::onTypingStarted);
        connect(m_groqClient, &GroqClient::typingFinished, this, &AIAssistantDialog::onTypingFinished);
        
        updateConnectionIndicator();
    }
}

void AIAssistantDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    animateDialogEntry();
    m_messageInput->setFocus();
}

void AIAssistantDialog::hideEvent(QHideEvent *event)
{
    QDialog::hideEvent(event);
    saveSettings();
}

void AIAssistantDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    scrollToBottom();
}

void AIAssistantDialog::sendMessage()
{
    QString message = m_messageInput->text().trimmed();
    if (message.isEmpty() || !m_groqClient) return;
    
    // Add user message to chat
    addMessage(ChatBubble::User, message);
    
    // Clear input
    m_messageInput->clear();
    m_sendButton->setEnabled(false);
    
    // Send to API
    m_groqClient->sendMessage(message, m_chatHistory);
    
    // Add user message to history
    m_chatHistory.append(ChatMessage("user", message));
}

void AIAssistantDialog::onMessageReceived(const QString &message, const QString &messageId)
{
    Q_UNUSED(messageId)
    
    // Add assistant message to chat
    addMessage(ChatBubble::Assistant, formatMessage(message));
    
    // Add to history
    m_chatHistory.append(ChatMessage("assistant", message));
    
    // Limit history size
    while (m_chatHistory.size() > 20) {
        m_chatHistory.removeFirst();
    }
}

void AIAssistantDialog::onErrorOccurred(const QString &error, int code)
{
    Q_UNUSED(code)
    
    QString errorMessage = QString("❌ Error: %1").arg(error);
    addMessage(ChatBubble::Assistant, errorMessage);
}

void AIAssistantDialog::onConnectionStatusChanged(bool connected)
{
    updateConnectionIndicator();
    
    if (connected) {
        addMessage(ChatBubble::System, "✅ Connected to AI service");
    } else {
        addMessage(ChatBubble::System, "❌ Disconnected from AI service");
    }
}

void AIAssistantDialog::onRequestStarted()
{
    m_progressBar->setVisible(true);
    m_sendButton->setEnabled(false);
}

void AIAssistantDialog::onRequestFinished()
{
    m_progressBar->setVisible(false);
    m_sendButton->setEnabled(!m_messageInput->text().trimmed().isEmpty());
}

void AIAssistantDialog::onTypingStarted()
{
    m_typingIndicator->show();
    scrollToBottom();
}

void AIAssistantDialog::onTypingFinished()
{
    m_typingIndicator->hide();
}

void AIAssistantDialog::addMessage(ChatBubble::Type type, const QString &message)
{
    ChatBubble *bubble = new ChatBubble(type, message);
    
    // Insert before the stretch and typing indicator
    int insertIndex = m_chatLayout->count() - 2;
    m_chatLayout->insertWidget(insertIndex, bubble);
    
    // Animate the bubble
    QTimer::singleShot(50, [bubble]() {
        bubble->animateIn();
    });
    
    // Auto-scroll to bottom
    if (m_autoScroll) {
        QTimer::singleShot(100, this, &AIAssistantDialog::scrollToBottom);
    }
}

void AIAssistantDialog::scrollToBottom()
{
    QScrollBar *scrollBar = m_chatScrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void AIAssistantDialog::updateConnectionIndicator()
{
    bool connected = m_groqClient && m_groqClient->isConnected();
    
    if (connected) {
        m_connectionIndicator->setStyleSheet("border-radius: 6px; background-color: #4CAF50;");
    } else {
        m_connectionIndicator->setStyleSheet("border-radius: 6px; background-color: #f44336;");
    }
}

void AIAssistantDialog::animateDialogEntry()
{
    setGraphicsEffect(m_opacityEffect);
    
    // Start small and grow
    QRect finalGeometry = geometry();
    QRect startGeometry = finalGeometry;
    startGeometry.setSize(QSize(finalGeometry.width() * 0.8, finalGeometry.height() * 0.8));
    startGeometry.moveCenter(finalGeometry.center());
    
    setGeometry(startGeometry);
    
    m_scaleAnimation->setStartValue(startGeometry);
    m_scaleAnimation->setEndValue(finalGeometry);
    
    m_fadeInAnimation->setStartValue(0.0);
    m_fadeInAnimation->setEndValue(1.0);
    
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(m_fadeInAnimation);
    group->addAnimation(m_scaleAnimation);
    
    connect(group, &QAbstractAnimation::finished, [this]() {
        setGraphicsEffect(nullptr);
    });
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

QString AIAssistantDialog::formatMessage(const QString &message)
{
    // Simple formatting for better readability
    QString formatted = message;
    
    // Convert bullet points
    QRegularExpression bulletRegex1("^\\* (.+)$", QRegularExpression::MultilineOption);
    formatted.replace(bulletRegex1, "• \\1");
    
    QRegularExpression bulletRegex2("^- (.+)$", QRegularExpression::MultilineOption);
    formatted.replace(bulletRegex2, "• \\1");
    
    return formatted;
}

void AIAssistantDialog::clearChat()
{
    // Remove all chat bubbles except welcome message
    while (m_chatLayout->count() > 3) { // Keep stretch, typing indicator, and welcome
        QLayoutItem *item = m_chatLayout->takeAt(0);
        if (item && item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    // Clear history
    m_chatHistory.clear();
    
    // Add welcome message back
    addMessage(ChatBubble::Assistant, 
               "👋 Chat cleared! How can I help you with materials management?");
}

void AIAssistantDialog::exportChat()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Chat",
        QString("AI_Chat_%1.txt").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss")),
        "Text Files (*.txt);;All Files (*)"
    );
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "ArchiFlow AI Assistant Chat Export\n";
            out << "Generated: " << QDateTime::currentDateTime().toString() << "\n";
            out << "========================================\n\n";
            
            for (const ChatMessage &msg : m_chatHistory) {
                out << QString("[%1] %2: %3\n\n")
                       .arg(QDateTime::fromMSecsSinceEpoch(msg.timestamp).toString("hh:mm:ss"))
                       .arg(msg.role.toUpper())
                       .arg(msg.content);
            }
            
            QMessageBox::information(this, "Export Complete", "Chat exported successfully!");
        } else {
            QMessageBox::warning(this, "Export Failed", "Could not write to file.");
        }
    }
}

void AIAssistantDialog::showSettings()
{
    // TODO: Implement settings dialog
    QMessageBox::information(this, "Settings", "Settings dialog coming soon!");
}

void AIAssistantDialog::onQuickActionClicked()
{
    QListWidgetItem *item = m_quickActionsList->currentItem();
    if (item) {
        QString action = item->text();
        
        // Handle special actions
        if (action.contains("Upload & analyze document")) {
            attachDocument();
            return;
        }
        
        // Remove emoji and send as message for other actions
        QString message = action.mid(2); // Remove emoji prefix
        m_messageInput->setText(message);
        sendMessage();
    }
}

void AIAssistantDialog::saveSettings()
{
    QSettings settings;
    settings.beginGroup("AIAssistant");
    settings.setValue("geometry", geometry());
    settings.setValue("autoScroll", m_autoScroll);
    settings.setValue("showTimestamps", m_showTimestamps);
    settings.setValue("fontFamily", m_fontFamily);
    settings.setValue("fontSize", m_fontSize);
    settings.endGroup();
}

void AIAssistantDialog::loadSettings()
{
    QSettings settings;
    settings.beginGroup("AIAssistant");
    
    QRect defaultGeometry(100, 100, 800, 600);
    setGeometry(settings.value("geometry", defaultGeometry).toRect());
    
    m_autoScroll = settings.value("autoScroll", true).toBool();
    m_showTimestamps = settings.value("showTimestamps", true).toBool();
    m_fontFamily = settings.value("fontFamily", "Poppins").toString();
    m_fontSize = settings.value("fontSize", 14).toInt();
    
    settings.endGroup();
}

void AIAssistantDialog::attachDocument()
{
    QString filter = "All Supported (*.pdf *.csv *.xlsx *.xls *.txt);;";
    filter += "PDF Documents (*.pdf);;";
    filter += "CSV Files (*.csv);;";
    filter += "Excel Files (*.xlsx *.xls);;";
    filter += "Text Files (*.txt);;";
    filter += "All Files (*)";
    
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Select Document to Analyze"),
        QString(),
        filter
    );
    
    if (!fileName.isEmpty()) {
        onDocumentSelected(fileName);
    }
}

void AIAssistantDialog::onDocumentSelected(const QString &filePath)
{
    if (!DocumentProcessor::isSupportedFormat(filePath)) {
        QMessageBox::warning(this, tr("Unsupported Format"), 
                           tr("The selected file format is not supported for analysis."));
        return;
    }
    
    // Show progress
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // Indeterminate progress
    
    // Add a message indicating document processing
    QFileInfo fileInfo(filePath);
    QString processingMessage = QString("📄 Processing document: %1\nFile type: %2\nFile size: %3")
                               .arg(fileInfo.fileName())
                               .arg(DocumentProcessor::getFileFormat(filePath))
                               .arg(DocumentProcessor::getFileSize(filePath));
    
    addMessage(ChatBubble::System, processingMessage);
    
    // Process document in a separate thread (simplified version - synchronous for now)
    analyzeDocument(filePath);
}

void AIAssistantDialog::analyzeDocument(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    
    try {
        // Extract content from document
        QString content = DocumentProcessor::extractText(filePath);
        
        if (content.isEmpty()) {
            addMessage(ChatBubble::System, "❌ Failed to extract content from the document.");
            m_progressBar->setVisible(false);
            return;
        }
        
        // Show document preview
        showDocumentPreview(fileName, content);
        
        // Generate analysis prompt
        QString analysisPrompt = generateAnalysisPrompt(fileName, content);
        
        // Send to AI for analysis
        if (m_groqClient && m_groqClient->isConnected()) {
            // Add user message showing the document was uploaded
            QString uploadMessage = QString("📎 Uploaded document: %1\n\nPlease analyze this document and provide insights about the materials, specifications, costs, or any other relevant information.")
                                   .arg(fileName);
            addMessage(ChatBubble::User, uploadMessage);
            
            // Send the analysis request to AI
            m_groqClient->sendMessage(analysisPrompt);
            
            // Show typing indicator
            m_typingIndicator->show();
        } else {
            addMessage(ChatBubble::System, "❌ AI service is not available. Please check your connection.");
        }
        
    } catch (const std::exception &e) {
        addMessage(ChatBubble::System, QString("❌ Error processing document: %1").arg(e.what()));
    }
    
    m_progressBar->setVisible(false);
}

QString AIAssistantDialog::generateAnalysisPrompt(const QString &fileName, const QString &content)
{
    QString prompt = QString(
        "You are an expert materials analyst for ArchiFlow, an architecture and construction management application. "
        "Please analyze the following document and provide comprehensive insights:\n\n"
        "Document: %1\n"
        "Content:\n%2\n\n"
        "Please provide analysis in the following areas:\n"
        "1. **Material Types**: Identify and categorize any materials mentioned\n"
        "2. **Specifications**: Technical specifications, standards, or requirements\n"
        "3. **Cost Analysis**: Any pricing information, cost estimates, or budget data\n"
        "4. **Quality & Compliance**: Quality standards, certifications, or regulatory compliance\n"
        "5. **Recommendations**: Professional recommendations for procurement or usage\n"
        "6. **Key Insights**: Important findings or observations\n\n"
        "Format your response with clear headings and bullet points for easy reading. "
        "Focus on actionable insights that would be valuable for construction and architecture professionals."
    ).arg(fileName).arg(content);
    
    return prompt;
}

void AIAssistantDialog::showDocumentPreview(const QString &fileName, const QString &content)
{
    // Truncate content for preview (first 500 characters)
    QString preview = content.left(500);
    if (content.length() > 500) {
        preview += "\n\n... (content truncated for preview)";
    }
    
    QString previewMessage = QString("📋 **Document Preview: %1**\n\n%2")
                            .arg(fileName)
                            .arg(preview);
    
    addMessage(ChatBubble::System, previewMessage);
}

void AIAssistantDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QString filePath = urls.first().toLocalFile();
            if (DocumentProcessor::isSupportedFormat(filePath)) {
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void AIAssistantDialog::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QString filePath = urls.first().toLocalFile();
            if (DocumentProcessor::isSupportedFormat(filePath)) {
                onDocumentSelected(filePath);
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void AIAssistantDialog::setDatabaseService(DatabaseService *dbService)
{
    m_databaseService = dbService;
      if (m_databaseService) {
        connect(m_databaseService, &DatabaseService::operationCompleted, 
                this, [this](const QString &operation, bool success, const QString &message) {
            Q_UNUSED(success)
            QString statusMessage = QString("%1: %2").arg(operation).arg(message);
            addMessage(ChatBubble::System, statusMessage);
        });
        
        connect(m_databaseService, &DatabaseService::dataChanged,
                this, [this]() {
            addMessage(ChatBubble::System, "Database updated. Current data context refreshed.");
        });
    }
}

void AIAssistantDialog::setMaterialContext(const QJsonObject &context)
{
    m_materialContext = context;
}

void AIAssistantDialog::processDatabaseCommand(const QString &command)
{
    if (!m_databaseService) {
        addMessage(ChatBubble::System, "Database service not available.");
        return;
    }
    
    addMessage(ChatBubble::System, QString("Processing database command: %1").arg(command));
    
    // Parse the command and execute appropriate database operations
    QString lowerCommand = command.toLower().trimmed();
    
    if (lowerCommand.startsWith("get all materials") || lowerCommand.startsWith("list all materials")) {
        QJsonArray materials = m_databaseService->getAllMaterials();
        QString response = QString("Found %1 materials in the database:\n").arg(materials.size());
        
        for (const auto &value : materials) {
            QJsonObject material = value.toObject();
            response += QString("- %1 (%2) - Qty: %3\n")
                       .arg(material["name"].toString())
                       .arg(material["category"].toString())
                       .arg(material["quantity"].toInt());
        }
        
        addMessage(ChatBubble::Assistant, response);
    }
    else if (lowerCommand.startsWith("get low stock") || lowerCommand.startsWith("low stock materials")) {
        QJsonArray lowStock = m_databaseService->getLowStockMaterials();
        QString response = QString("Low stock materials (%1):\n").arg(lowStock.size());
        
        for (const auto &value : lowStock) {
            QJsonObject material = value.toObject();
            response += QString("- %1: %2 (Reorder at: %3)\n")
                       .arg(material["name"].toString())
                       .arg(material["quantity"].toInt())
                       .arg(material["reorderPoint"].toInt());
        }
        
        addMessage(ChatBubble::Assistant, response);
    }
    else if (lowerCommand.startsWith("search ")) {
        QString searchTerm = command.mid(7).trimmed(); // Remove "search " prefix
        QJsonArray results = m_databaseService->searchMaterials(searchTerm);
        QString response = QString("Search results for '%1' (%2 found):\n").arg(searchTerm).arg(results.size());
        
        for (const auto &value : results) {
            QJsonObject material = value.toObject();
            response += QString("- %1 (%2) - %3\n")
                       .arg(material["name"].toString())
                       .arg(material["category"].toString())
                       .arg(material["description"].toString());
        }
        
        addMessage(ChatBubble::Assistant, response);
    }
    else {
        addMessage(ChatBubble::System, "Unknown database command. Try 'get all materials', 'get low stock', or 'search [term]'.");
    }
}

void AIAssistantDialog::handleDatabaseQuery(const QString &query)
{
    if (!m_databaseService) {
        addMessage(ChatBubble::System, "Database service not available.");
        return;
    }
    
    addMessage(ChatBubble::System, QString("Executing database query: %1").arg(query));
    
    // For security, we'll only allow certain safe queries
    QString lowerQuery = query.toLower().trimmed();
    
    if (lowerQuery.contains("delete") || lowerQuery.contains("drop") || lowerQuery.contains("truncate")) {
        addMessage(ChatBubble::System, "Destructive queries are not allowed for security reasons.");
        return;
    }
    
    // This is a simplified implementation - in a real app you'd want more sophisticated query parsing
    if (lowerQuery.startsWith("select") && lowerQuery.contains("materials")) {
        // For now, just redirect to search or general queries
        if (lowerQuery.contains("where")) {
            addMessage(ChatBubble::Assistant, "Complex SQL queries not yet supported. Try using the search command instead.");
        } else {
            processDatabaseCommand("get all materials");
        }
    } else {
        addMessage(ChatBubble::Assistant, "Only SELECT queries on materials table are currently supported.");
    }
}
