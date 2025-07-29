#include "contractchatbotdialog.h"
#include "../../interfaces/icontractchatbot.h"
#include "contractdatabasemanager.h"
#include "contract.h"
#include "utils/stylemanager.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextDocument>
#include <QDateTime>
#include <QScrollBar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QGroupBox>
#include <QStandardPaths>
#include <QUuid>

ContractChatbotDialog::ContractChatbotDialog(QWidget *parent)
    : BaseDialog(parent)
    , m_chatbot(nullptr)
    , m_dbManager(nullptr)
    , m_currentContract(nullptr)
    , m_currentChatMode("General")
    , m_currentAnalysisMode("Quick")
    , m_suggestionsEnabled(true)
    , m_analysisEnabled(true)
    , m_typingTimer(new QTimer(this))
    , m_suggestionTimer(new QTimer(this))
{
    setWindowTitle("ArchiFlow Contract AI Assistant");
    setWindowIcon(QIcon(":/icons/chatbot.png"));
    resize(900, 700);
    
    m_currentSessionId = QUuid::createUuid().toString();
    
    setupUi();
    setupConnections();
    applyArchiFlowStyling();
    
    // Configure timers
    m_typingTimer->setSingleShot(true);
    m_typingTimer->setInterval(TYPING_TIMEOUT);
    
    m_suggestionTimer->setSingleShot(true);
    m_suggestionTimer->setInterval(1000);
    
    startNewConversation();
}

ContractChatbotDialog::~ContractChatbotDialog()
{
    saveConversationHistory();
}

void ContractChatbotDialog::setupUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);

    m_mainSplitter = new QSplitter(Qt::Horizontal);
    
    setupChatArea();
    setupControlPanel();
    setupInputArea();
    
    // Add widgets to splitter
    m_mainSplitter->addWidget(m_chatWidget);
    m_mainSplitter->addWidget(m_controlPanel);
    m_mainSplitter->setSizes({600, 300});
    
    m_mainLayout->addWidget(m_mainSplitter);
    m_mainLayout->addWidget(m_inputWidget);
    
    // Status area
    m_statusWidget = new QWidget;
    QHBoxLayout *statusLayout = new QHBoxLayout(m_statusWidget);
    
    m_statusLabel = new QLabel("Ready");
    m_progressBar = new QProgressBar;
    m_progressBar->setVisible(false);
    
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(m_progressBar);
    
    m_mainLayout->addWidget(m_statusWidget);
}

void ContractChatbotDialog::setupChatArea()
{
    m_chatWidget = new QWidget;
    QVBoxLayout *chatLayout = new QVBoxLayout(m_chatWidget);
    
    // Chat header
    QLabel *headerLabel = new QLabel("Contract AI Assistant");
    headerLabel->setObjectName("chatHeader");
    headerLabel->setAlignment(Qt::AlignCenter);
    
    // Chat messages area
    m_chatScrollArea = new QScrollArea;
    m_chatScrollArea->setWidgetResizable(true);
    m_chatScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_chatScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_messagesContainer = new QWidget;
    m_messagesLayout = new QVBoxLayout(m_messagesContainer);
    m_messagesLayout->setSpacing(10);
    m_messagesLayout->addStretch();
    
    m_chatScrollArea->setWidget(m_messagesContainer);
    
    // Typing indicator
    m_typingIndicator = new QWidget;
    QHBoxLayout *typingLayout = new QHBoxLayout(m_typingIndicator);
    QLabel *typingLabel = new QLabel("AI is typing...");
    typingLabel->setObjectName("typingIndicator");
    typingLayout->addWidget(typingLabel);
    typingLayout->addStretch();
    m_typingIndicator->setVisible(false);
    
    chatLayout->addWidget(headerLabel);
    chatLayout->addWidget(m_chatScrollArea);
    chatLayout->addWidget(m_typingIndicator);
}

void ContractChatbotDialog::setupInputArea()
{
    m_inputWidget = new QWidget;
    QHBoxLayout *inputLayout = new QHBoxLayout(m_inputWidget);
    
    m_messageInput = new QLineEdit;
    m_messageInput->setPlaceholderText("Ask me anything about contracts...");
    m_messageInput->setMaxLength(MAX_MESSAGE_LENGTH);
    
    m_sendButton = new QPushButton("Send");
    m_sendButton->setObjectName("primaryButton");
    m_sendButton->setEnabled(false);
    
    m_clearButton = new QPushButton("Clear");
    m_clearButton->setIcon(QIcon(":/icons/clear.png"));
    
    m_exportButton = new QPushButton("Export");
    m_exportButton->setIcon(QIcon(":/icons/export.png"));
    
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendButton);
    inputLayout->addWidget(m_clearButton);
    inputLayout->addWidget(m_exportButton);
}

void ContractChatbotDialog::setupControlPanel()
{
    m_controlPanel = new QWidget;
    QVBoxLayout *controlLayout = new QVBoxLayout(m_controlPanel);
    
    // Chat settings
    QGroupBox *settingsGroup = new QGroupBox("Chat Settings");
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsGroup);
    
    // Chat mode
    QLabel *modeLabel = new QLabel("Chat Mode:");
    m_chatModeCombo = new QComboBox;
    m_chatModeCombo->addItems({"General", "Analysis", "Comparison", "Risk Assessment", "Legal Guidance"});
    
    // Analysis depth
    QLabel *analysisLabel = new QLabel("Analysis Depth:");
    m_analysisModeCombo = new QComboBox;
    m_analysisModeCombo->addItems({"Quick", "Standard", "Detailed", "Comprehensive"});
    
    // Options
    m_enableSuggestionsCheck = new QCheckBox("Enable Suggestions");
    m_enableSuggestionsCheck->setChecked(m_suggestionsEnabled);
    
    m_enableAnalysisCheck = new QCheckBox("Enable Auto-Analysis");
    m_enableAnalysisCheck->setChecked(m_analysisEnabled);
    
    m_settingsButton = new QPushButton("Advanced Settings");
    
    settingsLayout->addWidget(modeLabel);
    settingsLayout->addWidget(m_chatModeCombo);
    settingsLayout->addWidget(analysisLabel);
    settingsLayout->addWidget(m_analysisModeCombo);
    settingsLayout->addWidget(m_enableSuggestionsCheck);
    settingsLayout->addWidget(m_enableAnalysisCheck);
    settingsLayout->addWidget(m_settingsButton);
    
    // Quick actions
    QGroupBox *actionsGroup = new QGroupBox("Quick Actions");
    m_quickActionsLayout = new QVBoxLayout(actionsGroup);
    
    m_analyzeContractButton = new QPushButton("Analyze Current Contract");
    m_analyzeContractButton->setIcon(QIcon(":/icons/analyze.png"));
    m_analyzeContractButton->setEnabled(false);
    
    m_compareContractsButton = new QPushButton("Compare Contracts");
    m_compareContractsButton->setIcon(QIcon(":/icons/compare.png"));
    m_compareContractsButton->setEnabled(false);
    
    m_identifyRisksButton = new QPushButton("Identify Risks");
    m_identifyRisksButton->setIcon(QIcon(":/icons/warning.png"));
    m_identifyRisksButton->setEnabled(false);
    
    m_generateSummaryButton = new QPushButton("Generate Summary");
    m_generateSummaryButton->setIcon(QIcon(":/icons/summary.png"));
    m_generateSummaryButton->setEnabled(false);
    
    m_extractTermsButton = new QPushButton("Extract Key Terms");
    m_extractTermsButton->setIcon(QIcon(":/icons/terms.png"));
    m_extractTermsButton->setEnabled(false);
    
    m_quickActionsLayout->addWidget(m_analyzeContractButton);
    m_quickActionsLayout->addWidget(m_compareContractsButton);
    m_quickActionsLayout->addWidget(m_identifyRisksButton);
    m_quickActionsLayout->addWidget(m_generateSummaryButton);
    m_quickActionsLayout->addWidget(m_extractTermsButton);
    m_quickActionsLayout->addStretch();
    
    // Suggestions area
    QGroupBox *suggestionsGroup = new QGroupBox("Suggestions");
    m_suggestionsLayout = new QVBoxLayout(suggestionsGroup);
    
    m_suggestionsLabel = new QLabel("Type a message to see suggestions...");
    m_suggestionsLabel->setWordWrap(true);
    m_suggestionsLabel->setObjectName("suggestionsLabel");
    
    m_suggestionsLayout->addWidget(m_suggestionsLabel);
    m_suggestionsLayout->addStretch();
    
    // Add all groups to control panel
    controlLayout->addWidget(settingsGroup);
    controlLayout->addWidget(actionsGroup);
    controlLayout->addWidget(suggestionsGroup);
    controlLayout->addStretch();
}

void ContractChatbotDialog::setupConnections()
{
    // Input connections
    connect(m_messageInput, &QLineEdit::textChanged, this, &ContractChatbotDialog::onMessageChanged);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &ContractChatbotDialog::onSendClicked);
    connect(m_sendButton, &QPushButton::clicked, this, &ContractChatbotDialog::onSendClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &ContractChatbotDialog::clearChat);
    connect(m_exportButton, &QPushButton::clicked, this, &ContractChatbotDialog::exportChat);
    
    // Settings connections
    connect(m_chatModeCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &ContractChatbotDialog::onChatModeChanged);
    connect(m_analysisModeCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &ContractChatbotDialog::onAnalysisModeChanged);
    connect(m_enableSuggestionsCheck, &QCheckBox::toggled, [this](bool enabled) {
        m_suggestionsEnabled = enabled;
        updateSuggestions();
    });
    connect(m_enableAnalysisCheck, &QCheckBox::toggled, [this](bool enabled) {
        m_analysisEnabled = enabled;
    });
    connect(m_settingsButton, &QPushButton::clicked, this, &ContractChatbotDialog::onSettingsClicked);
    
    // Quick actions connections
    connect(m_analyzeContractButton, &QPushButton::clicked, this, &ContractChatbotDialog::analyzeCurrentContract);
    connect(m_compareContractsButton, &QPushButton::clicked, this, &ContractChatbotDialog::compareSelectedContracts);
    connect(m_identifyRisksButton, &QPushButton::clicked, [this]() {
        if (m_currentContract) {
            QString message = "Please identify potential risks in the current contract.";
            m_messageInput->setText(message);
            onSendClicked();
        }
    });
    connect(m_generateSummaryButton, &QPushButton::clicked, [this]() {
        if (m_currentContract) {
            QString message = "Please generate a summary of the current contract.";
            m_messageInput->setText(message);
            onSendClicked();
        }
    });
    connect(m_extractTermsButton, &QPushButton::clicked, [this]() {
        if (m_currentContract) {
            QString message = "Please extract key terms from the current contract.";
            m_messageInput->setText(message);
            onSendClicked();
        }
    });
    
    // Timer connections
    connect(m_typingTimer, &QTimer::timeout, this, &ContractChatbotDialog::onTypingTimerTimeout);
    connect(m_suggestionTimer, &QTimer::timeout, this, &ContractChatbotDialog::updateSuggestions);
}

void ContractChatbotDialog::setChatbot(IContractChatbot *chatbot)
{
    m_chatbot = chatbot;
    updateAnalysisOptions();
}

void ContractChatbotDialog::setDatabaseManager(ContractDatabaseManager *dbManager)
{
    m_dbManager = dbManager;
}

void ContractChatbotDialog::setCurrentContract(const Contract *contract)
{
    m_currentContract = contract;
    
    // Enable/disable quick action buttons
    bool hasContract = (contract != nullptr);
    m_analyzeContractButton->setEnabled(hasContract && m_chatbot && m_chatbot->isAvailable());
    m_identifyRisksButton->setEnabled(hasContract && m_chatbot && m_chatbot->isAvailable());
    m_generateSummaryButton->setEnabled(hasContract && m_chatbot && m_chatbot->isAvailable());
    m_extractTermsButton->setEnabled(hasContract && m_chatbot && m_chatbot->isAvailable());
    
    if (hasContract) {
        QString welcomeMessage = QString("I'm now analyzing contract: %1. How can I help you with this contract?")
            .arg(contract->clientName());
        addSystemMessage(welcomeMessage);
    }
}

void ContractChatbotDialog::setContracts(const QList<Contract*> &contracts)
{
    m_contracts = contracts;
    m_compareContractsButton->setEnabled(contracts.size() >= 2 && m_chatbot && m_chatbot->isAvailable());
}

void ContractChatbotDialog::startNewConversation()
{
    clearChat();
    m_currentSessionId = QUuid::createUuid().toString();
    
    QString welcomeMessage = "Welcome to the ArchiFlow Contract AI Assistant! I can help you with:\n\n"
                           "• Contract analysis and insights\n"
                           "• Risk identification\n"
                           "• Contract comparisons\n"
                           "• Legal guidance\n"
                           "• Key terms extraction\n\n"
                           "How can I assist you today?";
    
    addMessage("AI Assistant", welcomeMessage, false);
}

void ContractChatbotDialog::onSendClicked()
{
    QString message = m_messageInput->text().trimmed();
    if (message.isEmpty() || !m_chatbot) {
        return;
    }
    
    // Add user message to chat
    addMessage("You", message, true);
    m_messageInput->clear();
    m_sendButton->setEnabled(false);
    
    // Show typing indicator
    showTypingIndicator();
    enableInput(false);
    
    // Process message with chatbot
    processUserMessage(message);
}

void ContractChatbotDialog::processUserMessage(const QString &message)
{
    if (!m_chatbot || !m_chatbot->isAvailable()) {
        hideTypingIndicator();
        enableInput(true);
        addSystemMessage("AI Assistant is currently unavailable. Please try again later.");
        return;
    }
    
    m_statusLabel->setText("Processing your request...");
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // Indeterminate progress
    
    // Simulate async processing with QTimer (in real implementation, this would be async)
    QTimer::singleShot(2000, [this, message]() {
        try {
            QString response;
            
            // Process based on current chat mode
            if (m_currentChatMode == "Analysis" && m_currentContract) {
                response = m_chatbot->analyzeContract(m_currentContract);
            } else if (m_currentChatMode == "Comparison" && m_contracts.size() >= 2) {
                response = m_chatbot->compareContracts(m_contracts);
            } else if (m_currentChatMode == "Risk Assessment" && m_currentContract) {
                response = m_chatbot->identifyRisks(m_currentContract);
            } else {
                response = m_chatbot->processQuery(message);
            }
            
            displayBotResponse(response);
            
        } catch (const std::exception &e) {
            displayBotResponse(QString("I encountered an error while processing your request: %1").arg(e.what()));
        }
        
        hideTypingIndicator();
        enableInput(true);
        m_statusLabel->setText("Ready");
        m_progressBar->setVisible(false);
    });
}

void ContractChatbotDialog::displayBotResponse(const QString &response)
{
    addMessage("AI Assistant", response, false);
    
    // Update conversation history
    m_conversationHistory.append(QString("User: %1").arg(m_messageInput->text()));
    m_conversationHistory.append(QString("AI: %1").arg(response));
    
    // Limit history size
    while (m_conversationHistory.size() > MAX_HISTORY_ITEMS) {
        m_conversationHistory.removeFirst();
    }
    
    // Update suggestions if enabled
    if (m_suggestionsEnabled) {
        m_suggestionTimer->start();
    }
}

void ContractChatbotDialog::addMessage(const QString &sender, const QString &message, bool isUser)
{
    QWidget *messageWidget = createMessageWidget(sender, message, isUser);
    
    // Insert before the stretch item
    int insertIndex = m_messagesLayout->count() - 1;
    m_messagesLayout->insertWidget(insertIndex, messageWidget);
    
    // Scroll to bottom
    QTimer::singleShot(50, this, &ContractChatbotDialog::scrollToBottom);
}

void ContractChatbotDialog::addSystemMessage(const QString &message)
{
    addMessage("System", message, false);
}

QWidget* ContractChatbotDialog::createMessageWidget(const QString &sender, const QString &message, bool isUser)
{
    QWidget *messageWidget = new QWidget;
    messageWidget->setObjectName(isUser ? "userMessage" : "botMessage");
    
    QVBoxLayout *layout = new QVBoxLayout(messageWidget);
    layout->setContentsMargins(10, 8, 10, 8);
    
    // Sender label
    QLabel *senderLabel = new QLabel(sender);
    senderLabel->setObjectName("messageSender");
    senderLabel->setAlignment(isUser ? Qt::AlignRight : Qt::AlignLeft);
    
    // Message content
    QLabel *contentLabel = new QLabel(message);
    contentLabel->setWordWrap(true);
    contentLabel->setObjectName("messageContent");
    contentLabel->setAlignment(isUser ? Qt::AlignRight : Qt::AlignLeft);
    contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    
    // Timestamp
    QLabel *timestampLabel = new QLabel(QDateTime::currentDateTime().toString("hh:mm"));
    timestampLabel->setObjectName("messageTimestamp");
    timestampLabel->setAlignment(isUser ? Qt::AlignRight : Qt::AlignLeft);
    
    layout->addWidget(senderLabel);
    layout->addWidget(contentLabel);
    layout->addWidget(timestampLabel);
    
    // Set alignment for the entire widget
    if (isUser) {
        layout->setAlignment(Qt::AlignRight);
        messageWidget->setMaximumWidth(400);
    } else {
        layout->setAlignment(Qt::AlignLeft);
        messageWidget->setMaximumWidth(500);
    }
    
    return messageWidget;
}

void ContractChatbotDialog::showTypingIndicator()
{
    m_typingIndicator->setVisible(true);
    scrollToBottom();
}

void ContractChatbotDialog::hideTypingIndicator()
{
    m_typingIndicator->setVisible(false);
}

void ContractChatbotDialog::scrollToBottom()
{
    QScrollBar *scrollBar = m_chatScrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void ContractChatbotDialog::enableInput(bool enabled)
{
    m_messageInput->setEnabled(enabled);
    m_sendButton->setEnabled(enabled && !m_messageInput->text().trimmed().isEmpty());
}

void ContractChatbotDialog::onMessageChanged()
{
    bool hasText = !m_messageInput->text().trimmed().isEmpty();
    m_sendButton->setEnabled(hasText);
    
    // Update suggestions with delay
    if (m_suggestionsEnabled && hasText) {
        m_suggestionTimer->start();
    }
}

void ContractChatbotDialog::updateSuggestions()
{
    if (!m_suggestionsEnabled || !m_chatbot || !m_chatbot->isAvailable()) {
        m_suggestionsLabel->setText("Suggestions unavailable");
        return;
    }
    
    QString partialQuery = m_messageInput->text().trimmed();
    if (partialQuery.isEmpty()) {
        m_suggestionsLabel->setText("Type a message to see suggestions...");
        return;
    }
    
    try {
        QStringList suggestions = m_chatbot->getSuggestions(partialQuery);
        if (suggestions.isEmpty()) {
            m_suggestionsLabel->setText("No suggestions available");
        } else {
            QString suggestionsText = suggestions.join("\n• ");
            m_suggestionsLabel->setText("• " + suggestionsText);
        }
    } catch (const std::exception &e) {
        m_suggestionsLabel->setText("Error getting suggestions");
    }
}

void ContractChatbotDialog::clearChat()
{
    // Clear all messages except the stretch item
    while (m_messagesLayout->count() > 1) {
        QLayoutItem *item = m_messagesLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    m_conversationHistory.clear();
    m_statusLabel->setText("Chat cleared");
}

void ContractChatbotDialog::exportChat()
{
    if (m_conversationHistory.isEmpty()) {
        QMessageBox::information(this, "Export Chat", "No conversation to export.");
        return;
    }
    
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString fileName = QString("archiflow_chat_%1.txt")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    
    QString filePath = QFileDialog::getSaveFileName(this, "Export Chat",
        QDir(defaultPath).filePath(fileName),
        "Text Files (*.txt);;All Files (*)");
    
    if (filePath.isEmpty()) {
        return;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Export Error", 
            QString("Could not save file: %1").arg(file.errorString()));
        return;
    }
    
    QTextStream out(&file);
    out << "ArchiFlow Contract AI Assistant Conversation\n";
    out << "Session ID: " << m_currentSessionId << "\n";
    out << "Exported: " << QDateTime::currentDateTime().toString() << "\n";
    out << QString("=").repeated(50) << "\n\n";
    
    for (const QString &entry : m_conversationHistory) {
        out << entry << "\n\n";
    }
    
    file.close();
    
    m_statusLabel->setText(QString("Chat exported to: %1").arg(filePath));
    emit chatExported(filePath);
}

void ContractChatbotDialog::analyzeCurrentContract()
{
    if (!m_currentContract || !m_chatbot) {
        return;
    }
    
    QString message = QString("Please provide a comprehensive analysis of the contract for client: %1")
        .arg(m_currentContract->clientName());
    
    m_messageInput->setText(message);
    onSendClicked();
}

void ContractChatbotDialog::compareSelectedContracts()
{
    if (m_contracts.size() < 2 || !m_chatbot) {
        return;
    }
    
    QStringList contractNames;
    for (const Contract *contract : m_contracts) {
        contractNames.append(contract->clientName());
    }
    
    QString message = QString("Please compare these contracts: %1")
        .arg(contractNames.join(", "));
    
    m_messageInput->setText(message);
    onSendClicked();
}

void ContractChatbotDialog::onChatModeChanged()
{
    m_currentChatMode = m_chatModeCombo->currentText();
    
    QString modeMessage = QString("Chat mode changed to: %1").arg(m_currentChatMode);
    addSystemMessage(modeMessage);
}

void ContractChatbotDialog::onAnalysisModeChanged()
{
    m_currentAnalysisMode = m_analysisModeCombo->currentText();
    
    if (m_chatbot) {
        // Map analysis mode to depth level
        int depth = 1;
        if (m_currentAnalysisMode == "Standard") depth = 2;
        else if (m_currentAnalysisMode == "Detailed") depth = 3;
        else if (m_currentAnalysisMode == "Comprehensive") depth = 5;
        
        m_chatbot->setAnalysisDepth(depth);
    }
}

void ContractChatbotDialog::onSettingsClicked()
{
    // TODO: Implement advanced settings dialog
    QMessageBox::information(this, "Settings", "Advanced settings dialog will be implemented in a future update.");
}

void ContractChatbotDialog::onTypingTimerTimeout()
{
    hideTypingIndicator();
}

void ContractChatbotDialog::updateAnalysisOptions()
{
    bool available = m_chatbot && m_chatbot->isAvailable();
    
    m_analyzeContractButton->setEnabled(available && m_currentContract);
    m_compareContractsButton->setEnabled(available && m_contracts.size() >= 2);
    m_identifyRisksButton->setEnabled(available && m_currentContract);
    m_generateSummaryButton->setEnabled(available && m_currentContract);
    m_extractTermsButton->setEnabled(available && m_currentContract);
    
    if (available) {
        QStringList features = m_chatbot->getAvailableFeatures();
        QString statusText = QString("AI Available - Features: %1").arg(features.join(", "));
        m_statusLabel->setText(statusText);
    } else {
        m_statusLabel->setText("AI Assistant unavailable");
    }
}

void ContractChatbotDialog::applyArchiFlowStyling()
{
    // Apply custom styling for ArchiFlow theme
    QString styles = R"(
        QWidget#chatHeader {
            font-size: 16px;
            font-weight: bold;
            color: #2c3e50;
            padding: 10px;
            background-color: #ecf0f1;
            border-radius: 5px;
            margin-bottom: 10px;
        }
        
        QWidget#userMessage {
            background-color: #3498db;
            color: white;
            border-radius: 10px;
            margin: 5px 50px 5px 20px;
        }
        
        QWidget#botMessage {
            background-color: #ecf0f1;
            color: #2c3e50;
            border-radius: 10px;
            margin: 5px 20px 5px 50px;
        }
        
        QLabel#messageSender {
            font-weight: bold;
            font-size: 12px;
        }
        
        QLabel#messageContent {
            font-size: 14px;
            line-height: 1.4;
        }
        
        QLabel#messageTimestamp {
            font-size: 10px;
            color: #7f8c8d;
        }
        
        QWidget#typingIndicator {
            background-color: #f8f9fa;
            color: #6c757d;
            border-radius: 5px;
            padding: 5px 10px;
            font-style: italic;
        }
        
        QLabel#suggestionsLabel {
            font-size: 12px;
            color: #6c757d;
            background-color: #f8f9fa;
            padding: 10px;
            border-radius: 5px;
        }
    )";
    
    setStyleSheet(styles);
}

void ContractChatbotDialog::showEvent(QShowEvent *event)
{
    BaseDialog::showEvent(event);
    m_messageInput->setFocus();
    updateAnalysisOptions();
}

void ContractChatbotDialog::closeEvent(QCloseEvent *event)
{
    saveConversationHistory();
    BaseDialog::closeEvent(event);
}

void ContractChatbotDialog::saveConversationHistory()
{
    // TODO: Implement conversation history persistence
    // This could save to database or file for later retrieval
}

void ContractChatbotDialog::loadConversationHistory(const QString &sessionId)
{
    Q_UNUSED(sessionId)
    // TODO: Implement conversation history loading
    // This could load from database or file
}

void ContractChatbotDialog::sendMessage()
{
    QString message = m_messageInput->text().trimmed();
    if (message.isEmpty()) {
        return;
    }
    
    // Clear input
    m_messageInput->clear();
      // Add user message to chat
    addMessage("User", message, true);
    
    // Show typing indicator
    showTypingIndicator();
    
    // Process message with chatbot
    if (m_chatbot) {
        QString response = m_chatbot->processQuery(message);
        addMessage("Assistant", response, false);
    } else {
        addMessage("System", "Chatbot service is not available. Please check your configuration.", false);
    }
    
    hideTypingIndicator();
}

void ContractChatbotDialog::onSuggestionClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) {
        return;
    }
    
    QString suggestion = button->text();
    m_messageInput->setText(suggestion);
    m_messageInput->setFocus();
}
