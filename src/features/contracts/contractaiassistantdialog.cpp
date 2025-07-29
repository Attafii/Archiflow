#include "contractaiassistantdialog.h"
#include "contract.h"
#include "contractdatabasemanager.h"
#include "../../database/databaseservice.h"
#include <QApplication>
#include <QScreen>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTextDocument>
#include <QScrollBar>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QSplitter>

// ContractChatBubble Implementation
ContractChatBubble::ContractChatBubble(Type type, const QString &message, QWidget *parent)
    : QFrame(parent)
    , m_type(type)
    , m_message(message)
    , m_messageLabel(nullptr)
    , m_timestampLabel(nullptr)
    , m_avatarLabel(nullptr)
    , m_fadeInAnimation(nullptr)
    , m_slideInAnimation(nullptr)
    , m_typingTimer(new QTimer(this))
    , m_opacityEffect(new QGraphicsOpacityEffect(this))
    , m_typingDots(0)
{
    setupUI();
    setupAnimations();
}

void ContractChatBubble::setupUI()
{
    setFrameStyle(QFrame::Box);
    setLineWidth(1);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 8);
    mainLayout->setSpacing(8);
    
    // Avatar
    m_avatarLabel = new QLabel();
    m_avatarLabel->setFixedSize(32, 32);
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    m_avatarLabel->setStyleSheet("border-radius: 16px; background-color: #4A90E2; color: white; font-weight: bold;");
    
    if (m_type == User) {
        m_avatarLabel->setText("U");
        m_avatarLabel->setStyleSheet("border-radius: 16px; background-color: #E3C6B0; color: #2C1810; font-weight: bold;");
    } else if (m_type == Assistant) {
        m_avatarLabel->setText("AI");
        m_avatarLabel->setStyleSheet("border-radius: 16px; background-color: #4A90E2; color: white; font-weight: bold;");
    } else {
        m_avatarLabel->setText("S");
        m_avatarLabel->setStyleSheet("border-radius: 16px; background-color: #666; color: white; font-weight: bold;");
    }
    
    // Message content
    QVBoxLayout *contentLayout = new QVBoxLayout();
    
    m_messageLabel = new QLabel();
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    m_messageLabel->setOpenExternalLinks(true);
    setMessage(m_message);
    
    m_timestampLabel = new QLabel();
    m_timestampLabel->setStyleSheet("color: #666; font-size: 10px;");
    setTimestamp(QDateTime::currentDateTime());
    
    contentLayout->addWidget(m_messageLabel);
    contentLayout->addWidget(m_timestampLabel, 0, Qt::AlignRight);
    
    if (m_type == User) {
        mainLayout->addLayout(contentLayout);
        mainLayout->addWidget(m_avatarLabel);
        setStyleSheet("background-color: #F5F5F5; border: 1px solid #E0E0E0; border-radius: 8px;");
    } else {
        mainLayout->addWidget(m_avatarLabel);
        mainLayout->addLayout(contentLayout);
        setStyleSheet("background-color: #E8F4FD; border: 1px solid #B3D9FF; border-radius: 8px;");
    }
    
    setGraphicsEffect(m_opacityEffect);
    m_opacityEffect->setOpacity(0.0);
}

void ContractChatBubble::setupAnimations()
{
    m_fadeInAnimation = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_fadeInAnimation->setDuration(300);
    m_fadeInAnimation->setStartValue(0.0);
    m_fadeInAnimation->setEndValue(1.0);
    
    connect(m_typingTimer, &QTimer::timeout, this, [this]() {
        m_typingDots = (m_typingDots + 1) % 4;
        QString dots = QString(".").repeated(m_typingDots);
        QString typing = QString("Typing%1").arg(dots);
        m_messageLabel->setText(typing);
    });
}

void ContractChatBubble::setMessage(const QString &message)
{
    m_message = message;
    if (m_messageLabel) {
        m_messageLabel->setText(message);
    }
}

void ContractChatBubble::setTimestamp(const QDateTime &timestamp)
{
    if (m_timestampLabel) {
        m_timestampLabel->setText(timestamp.toString("hh:mm"));
    }
}

void ContractChatBubble::animateIn()
{
    if (m_fadeInAnimation) {
        m_fadeInAnimation->start();
    }
}

void ContractChatBubble::startTypingAnimation()
{
    m_typingTimer->start(500);
}

void ContractChatBubble::stopTypingAnimation()
{
    m_typingTimer->stop();
    setMessage(m_message);
}

// ContractTypingIndicator Implementation
ContractTypingIndicator::ContractTypingIndicator(QWidget *parent)
    : QWidget(parent)
    , m_layout(new QHBoxLayout(this))
    , m_animationTimer(new QTimer(this))
    , m_currentDot(0)
{
    setupUI();
    connect(m_animationTimer, &QTimer::timeout, this, &ContractTypingIndicator::updateAnimation);
}

void ContractTypingIndicator::setupUI()
{
    setFixedHeight(30);
    m_layout->setContentsMargins(16, 8, 16, 8);
    m_layout->setSpacing(4);
    
    QLabel *aiLabel = new QLabel("AI");
    aiLabel->setStyleSheet("background-color: #4A90E2; color: white; border-radius: 12px; padding: 4px 8px; font-weight: bold;");
    m_layout->addWidget(aiLabel);
    
    m_layout->addWidget(new QLabel("is typing"));
    
    m_dot1 = new QLabel("●");
    m_dot2 = new QLabel("●");
    m_dot3 = new QLabel("●");
    
    m_layout->addWidget(m_dot1);
    m_layout->addWidget(m_dot2);
    m_layout->addWidget(m_dot3);
    m_layout->addStretch();
    
    setStyleSheet("color: #666;");
}

void ContractTypingIndicator::show()
{
    QWidget::show();
    m_animationTimer->start(500);
}

void ContractTypingIndicator::hide()
{
    m_animationTimer->stop();
    QWidget::hide();
}

void ContractTypingIndicator::updateAnimation()
{
    m_dot1->setStyleSheet(m_currentDot == 0 ? "color: #4A90E2;" : "color: #CCC;");
    m_dot2->setStyleSheet(m_currentDot == 1 ? "color: #4A90E2;" : "color: #CCC;");
    m_dot3->setStyleSheet(m_currentDot == 2 ? "color: #4A90E2;" : "color: #CCC;");
    m_currentDot = (m_currentDot + 1) % 3;
}

// ContractAIAssistantDialog Implementation
ContractAIAssistantDialog::ContractAIAssistantDialog(QWidget *parent)
    : QDialog(parent)
    , m_groqClient(nullptr)
    , m_databaseService(nullptr)
    , m_contractDbManager(nullptr)
    , m_currentContract(nullptr)
    , m_autoScroll(true)
    , m_showTimestamps(true)
    , m_fontFamily("Segoe UI")
    , m_fontSize(10)
{
    setWindowTitle("Contract AI Assistant");
    setWindowIcon(QIcon(":/icons/ai_assistant.png"));
    setModal(false);
    setAttribute(Qt::WA_DeleteOnClose, false);
    
    // Set dialog size
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        int width = qMin(1000, screenGeometry.width() * 3 / 4);
        int height = qMin(700, screenGeometry.height() * 3 / 4);
        resize(width, height);
        
        // Center on screen
        move((screenGeometry.width() - width) / 2, (screenGeometry.height() - height) / 2);
    }
    
    setupUI();
    setupAnimations();
    setupConnections();
    setupQuickActions();
    applyArchiFlowTheme();
    loadSettings();
}

ContractAIAssistantDialog::~ContractAIAssistantDialog()
{
    saveSettings();
}

void ContractAIAssistantDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Header
    m_headerFrame = new QFrame();
    m_headerFrame->setObjectName("headerFrame");
    m_headerFrame->setFixedHeight(50);
    
    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerFrame);
    headerLayout->setContentsMargins(16, 8, 16, 8);
    
    m_titleLabel = new QLabel("Contract AI Assistant");
    m_titleLabel->setObjectName("titleLabel");
    
    m_connectionIndicator = new QLabel("●");
    m_connectionIndicator->setObjectName("connectionIndicator");
    
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(new QLabel("Status:"));
    headerLayout->addWidget(m_connectionIndicator);
    
    m_settingsButton = new QPushButton("⚙");
    m_closeButton = new QPushButton("✕");
    headerLayout->addWidget(m_settingsButton);
    headerLayout->addWidget(m_closeButton);
    
    // Main content area
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);
    
    // Chat area
    QWidget *chatContainer = new QWidget();
    QVBoxLayout *chatContainerLayout = new QVBoxLayout(chatContainer);
    chatContainerLayout->setContentsMargins(0, 0, 0, 0);
    
    m_chatScrollArea = new QScrollArea();
    m_chatScrollArea->setWidgetResizable(true);
    m_chatScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_chatScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_chatWidget = new QWidget();
    m_chatLayout = new QVBoxLayout(m_chatWidget);
    m_chatLayout->setContentsMargins(8, 8, 8, 8);
    m_chatLayout->setSpacing(8);
    m_chatLayout->addStretch();
    
    m_chatScrollArea->setWidget(m_chatWidget);
    
    // Typing indicator
    m_typingIndicator = new ContractTypingIndicator();
    m_typingIndicator->hide();
    
    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 0);
    m_progressBar->hide();
    
    // Input area
    m_inputFrame = new QFrame();
    m_inputFrame->setObjectName("inputFrame");
    QHBoxLayout *inputLayout = new QHBoxLayout(m_inputFrame);
    inputLayout->setContentsMargins(8, 8, 8, 8);
    
    m_messageInput = new QLineEdit();
    m_messageInput->setPlaceholderText("Ask about contracts, request analysis, or get guidance...");
    
    m_attachButton = new QPushButton("📎");
    m_sendButton = new QPushButton("Send");
    m_sendButton->setDefault(true);
    
    inputLayout->addWidget(m_attachButton);
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendButton);
    
    chatContainerLayout->addWidget(m_chatScrollArea);
    chatContainerLayout->addWidget(m_typingIndicator);
    chatContainerLayout->addWidget(m_progressBar);
    chatContainerLayout->addWidget(m_inputFrame);
    
    // Sidebar
    m_sidebarFrame = new QFrame();
    m_sidebarFrame->setObjectName("sidebarFrame");
    m_sidebarFrame->setFixedWidth(250);
    
    m_sidebarLayout = new QVBoxLayout(m_sidebarFrame);
    m_sidebarLayout->setContentsMargins(8, 8, 8, 8);
    
    m_quickActionsLabel = new QLabel("Quick Actions");
    m_quickActionsLabel->setObjectName("sectionLabel");
    
    m_quickActionsList = new QListWidget();
    m_quickActionsList->setObjectName("quickActionsList");
    
    m_sidebarLayout->addWidget(m_quickActionsLabel);
    m_sidebarLayout->addWidget(m_quickActionsList);
    m_sidebarLayout->addStretch();
    
    mainSplitter->addWidget(chatContainer);
    mainSplitter->addWidget(m_sidebarFrame);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 0);
    
    m_mainLayout->addWidget(m_headerFrame);
    m_mainLayout->addWidget(mainSplitter);
    
    // Set up graphics effects for animations
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(m_opacityEffect);
}

void ContractAIAssistantDialog::setupConnections()
{
    connect(m_sendButton, &QPushButton::clicked, this, &ContractAIAssistantDialog::sendMessage);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &ContractAIAssistantDialog::sendMessage);
    connect(m_attachButton, &QPushButton::clicked, this, &ContractAIAssistantDialog::attachDocument);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::close);
    connect(m_settingsButton, &QPushButton::clicked, this, &ContractAIAssistantDialog::showSettings);
    connect(m_quickActionsList, &QListWidget::itemClicked, this, &ContractAIAssistantDialog::onQuickActionClicked);
}

void ContractAIAssistantDialog::setupQuickActions()
{
    m_quickActions = {
        "Analyze current contract for risks",
        "Generate contract summary",
        "Extract key terms and clauses",
        "Compare payment terms",
        "Identify missing provisions",
        "Check compliance requirements",
        "Recommend contract improvements",
        "Explain legal terminology",
        "Calculate project timeline",
        "Assess financial obligations"
    };
    
    addContractQuickActions();
}

void ContractAIAssistantDialog::addContractQuickActions()
{
    m_quickActionsList->clear();
    for (const QString &action : m_quickActions) {
        m_quickActionsList->addItem(action);
    }
}

void ContractAIAssistantDialog::setupAnimations()
{
    m_fadeInAnimation = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_fadeInAnimation->setDuration(300);
    m_fadeInAnimation->setStartValue(0.0);
    m_fadeInAnimation->setEndValue(1.0);
}

void ContractAIAssistantDialog::setGroqClient(GroqClient *client)
{
    if (m_groqClient) {
        disconnect(m_groqClient, nullptr, this, nullptr);
    }
    
    m_groqClient = client;
    
    if (m_groqClient) {
        connect(m_groqClient, &GroqClient::messageReceived, this, &ContractAIAssistantDialog::onMessageReceived);
        connect(m_groqClient, &GroqClient::errorOccurred, this, &ContractAIAssistantDialog::onErrorOccurred);
        connect(m_groqClient, &GroqClient::connectionStatusChanged, this, &ContractAIAssistantDialog::onConnectionStatusChanged);
        connect(m_groqClient, &GroqClient::requestStarted, this, &ContractAIAssistantDialog::onRequestStarted);
        connect(m_groqClient, &GroqClient::requestFinished, this, &ContractAIAssistantDialog::onRequestFinished);
        connect(m_groqClient, &GroqClient::typingStarted, this, &ContractAIAssistantDialog::onTypingStarted);
        connect(m_groqClient, &GroqClient::typingFinished, this, &ContractAIAssistantDialog::onTypingFinished);
        
        updateConnectionIndicator();
    }
}

void ContractAIAssistantDialog::setDatabaseService(DatabaseService *dbService)
{
    m_databaseService = dbService;
}

void ContractAIAssistantDialog::setContractDatabaseManager(ContractDatabaseManager *dbManager)
{
    m_contractDbManager = dbManager;
}

void ContractAIAssistantDialog::setContractContext(const QJsonObject &context)
{
    m_contractContext = context;
}

void ContractAIAssistantDialog::setCurrentContract(const Contract *contract)
{
    m_currentContract = contract;
}

void ContractAIAssistantDialog::sendMessage()
{
    QString message = m_messageInput->text().trimmed();
    if (message.isEmpty() || !m_groqClient) {
        return;
    }
    
    m_messageInput->clear();
    
    // Add user message to chat
    addMessage(ContractChatBubble::User, message);
    
    // Process the message with contract context
    QString processedMessage = processUserMessage(message);
    
    // Send to Groq
    QList<ChatMessage> context;
    if (m_currentContract) {
        QString contractInfo = QString(
            "Current contract context:\n"
            "Client: %1\n"
            "Value: $%2\n"
            "Status: %3\n"
            "Start Date: %4\n"
            "End Date: %5\n"
        ).arg(m_currentContract->clientName())
         .arg(m_currentContract->value(), 0, 'f', 2)
         .arg(m_currentContract->status())
         .arg(m_currentContract->startDate().toString("yyyy-MM-dd"))
         .arg(m_currentContract->endDate().toString("yyyy-MM-dd"));
        
        context.append(ChatMessage("system", contractInfo));
    }
    
    m_groqClient->sendMessage(processedMessage, context);
}

QString ContractAIAssistantDialog::processUserMessage(const QString &message)
{
    // Add contract-specific context to the message
    QString processedMessage = message;
    
    if (m_currentContract) {
        processedMessage = QString("Regarding the contract with %1: %2")
                          .arg(m_currentContract->clientName(), message);
    }
    
    return processedMessage;
}

void ContractAIAssistantDialog::addMessage(ContractChatBubble::Type type, const QString &message)
{
    ContractChatBubble *bubble = new ContractChatBubble(type, message);
    
    // Insert before the stretch
    int index = m_chatLayout->count() - 1;
    m_chatLayout->insertWidget(index, bubble);
    
    bubble->animateIn();
    
    if (m_autoScroll) {
        QTimer::singleShot(50, this, &ContractAIAssistantDialog::scrollToBottom);
    }
}

void ContractAIAssistantDialog::scrollToBottom()
{
    QScrollBar *scrollBar = m_chatScrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void ContractAIAssistantDialog::onMessageReceived(const QString &message, const QString &messageId)
{
    Q_UNUSED(messageId)
    addMessage(ContractChatBubble::Assistant, message);
}

void ContractAIAssistantDialog::onErrorOccurred(const QString &error, int code)
{
    Q_UNUSED(code)
    addMessage(ContractChatBubble::System, QString("Error: %1").arg(error));
}

void ContractAIAssistantDialog::onConnectionStatusChanged(bool connected)
{
    updateConnectionIndicator();
    if (!connected) {
        addMessage(ContractChatBubble::System, "Connection lost. Please check your internet connection and API key.");
    }
}

void ContractAIAssistantDialog::onRequestStarted()
{
    m_sendButton->setEnabled(false);
    m_progressBar->show();
}

void ContractAIAssistantDialog::onRequestFinished()
{
    m_sendButton->setEnabled(true);
    m_progressBar->hide();
}

void ContractAIAssistantDialog::onTypingStarted()
{
    m_typingIndicator->show();
    scrollToBottom();
}

void ContractAIAssistantDialog::onTypingFinished()
{
    m_typingIndicator->hide();
}

void ContractAIAssistantDialog::updateConnectionIndicator()
{
    if (m_groqClient && m_groqClient->isConnected()) {
        m_connectionIndicator->setText("●");
        m_connectionIndicator->setStyleSheet("color: #4CAF50;");
        m_connectionIndicator->setToolTip("Connected");
    } else {
        m_connectionIndicator->setText("●");
        m_connectionIndicator->setStyleSheet("color: #F44336;");
        m_connectionIndicator->setToolTip("Disconnected");
    }
}

void ContractAIAssistantDialog::onQuickActionClicked()
{
    QListWidgetItem *item = m_quickActionsList->currentItem();
    if (item) {
        m_messageInput->setText(item->text());
        sendMessage();
    }
}

void ContractAIAssistantDialog::clearChat()
{
    // Remove all chat bubbles except the stretch
    while (m_chatLayout->count() > 1) {
        QLayoutItem *item = m_chatLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    m_chatHistory.clear();
}

void ContractAIAssistantDialog::exportChat()
{
    // Implementation for exporting chat history
    QString fileName = QFileDialog::getSaveFileName(this, "Export Chat", "", "Text Files (*.txt)");
    if (!fileName.isEmpty()) {
        // Export logic here
    }
}

void ContractAIAssistantDialog::showSettings()
{
    // Implementation for settings dialog
    QMessageBox::information(this, "Settings", "Settings dialog would open here");
}

void ContractAIAssistantDialog::attachDocument()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Attach Document", "", "All Files (*)");
    if (!fileName.isEmpty()) {
        onDocumentSelected(fileName);
    }
}

void ContractAIAssistantDialog::onDocumentSelected(const QString &filePath)
{
    Q_UNUSED(filePath)
    // Implementation for document attachment
}

void ContractAIAssistantDialog::analyzeDocument(const QString &filePath)
{
    Q_UNUSED(filePath)
    // Implementation for document analysis
}

void ContractAIAssistantDialog::processDatabaseCommand(const QString &command)
{
    Q_UNUSED(command)
    // Implementation for database commands
}

void ContractAIAssistantDialog::handleDatabaseQuery(const QString &query)
{
    Q_UNUSED(query)
    // Implementation for database queries
}

void ContractAIAssistantDialog::applyArchiFlowTheme()
{
    setStyleSheet(R"(
        ContractAIAssistantDialog {
            background-color: #FAFAFA;
        }
        #headerFrame {
            background-color: #2C1810;
            border-bottom: 2px solid #E3C6B0;
        }
        #titleLabel {
            color: #E3C6B0;
            font-size: 16px;
            font-weight: bold;
        }
        #inputFrame {
            background-color: white;
            border-top: 1px solid #E0E0E0;
        }
        #sidebarFrame {
            background-color: #F5F5F5;
            border-left: 1px solid #E0E0E0;
        }
        #sectionLabel {
            font-weight: bold;
            color: #2C1810;
            margin-bottom: 8px;
        }
        #quickActionsList {
            border: 1px solid #E0E0E0;
            border-radius: 4px;
            background-color: white;
        }
        QPushButton {
            background-color: #E3C6B0;
            color: #2C1810;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #D4B896;
        }
        QPushButton:pressed {
            background-color: #C5AA85;
        }
        QLineEdit {
            border: 2px solid #E0E0E0;
            border-radius: 4px;
            padding: 8px;
            font-size: 12px;
        }
        QLineEdit:focus {
            border-color: #E3C6B0;
        }
    )");
}

void ContractAIAssistantDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    animateDialogEntry();
    m_messageInput->setFocus();
}

void ContractAIAssistantDialog::hideEvent(QHideEvent *event)
{
    QDialog::hideEvent(event);
    saveSettings();
}

void ContractAIAssistantDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (m_autoScroll) {
        QTimer::singleShot(50, this, &ContractAIAssistantDialog::scrollToBottom);
    }
}

void ContractAIAssistantDialog::animateDialogEntry()
{
    if (m_fadeInAnimation) {
        m_fadeInAnimation->start();
    }
}

void ContractAIAssistantDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void ContractAIAssistantDialog::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        const QList<QUrl> urls = mimeData->urls();
        for (const QUrl &url : urls) {
            if (url.isLocalFile()) {
                onDocumentSelected(url.toLocalFile());
            }
        }
    }
}

QString ContractAIAssistantDialog::formatMessage(const QString &message)
{
    return message;
}

void ContractAIAssistantDialog::saveSettings()
{
    QSettings settings;
    settings.beginGroup("ContractAIAssistant");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("autoScroll", m_autoScroll);
    settings.setValue("showTimestamps", m_showTimestamps);
    settings.setValue("fontFamily", m_fontFamily);
    settings.setValue("fontSize", m_fontSize);
    settings.endGroup();
}

void ContractAIAssistantDialog::loadSettings()
{
    QSettings settings;
    settings.beginGroup("ContractAIAssistant");
    restoreGeometry(settings.value("geometry").toByteArray());
    m_autoScroll = settings.value("autoScroll", true).toBool();
    m_showTimestamps = settings.value("showTimestamps", true).toBool();
    m_fontFamily = settings.value("fontFamily", "Segoe UI").toString();
    m_fontSize = settings.value("fontSize", 10).toInt();
    settings.endGroup();
}

QString ContractAIAssistantDialog::generateAnalysisPrompt(const QString &fileName, const QString &content)
{
    Q_UNUSED(fileName)
    Q_UNUSED(content)
    return QString();
}

void ContractAIAssistantDialog::showDocumentPreview(const QString &fileName, const QString &content)
{
    Q_UNUSED(fileName)
    Q_UNUSED(content)
}

QString ContractAIAssistantDialog::handleDatabaseOperation(const QString &operation, const QJsonObject &params)
{
    Q_UNUSED(operation)
    Q_UNUSED(params)
    return QString();
}
