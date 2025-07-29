#include "invoiceaiassistantdialog.h"
#include "invoice.h"
#include "client.h"
#include "invoicedatabasemanager.h"
#include "../materials/groqclient.h"

#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QClipboard>
#include <QScrollBar>
#include <QTextStream>
#include <QFile>
#include <QMimeDatabase>
#include <QTextDocumentFragment>
#include <QHBoxLayout>

// InvoiceChatBubble implementation
InvoiceChatBubble::InvoiceChatBubble(Type type, const QString &message, QWidget *parent)
    : QFrame(parent), m_type(type), m_message(message)
{
    setupUI();
    setupAnimations();
}

void InvoiceChatBubble::setMessage(const QString &message)
{
    m_message = message;
    m_messageLabel->setText(message);
}

void InvoiceChatBubble::setTimestamp(const QDateTime &timestamp)
{
    m_timestamp = timestamp;
    m_timestampLabel->setText(timestamp.toString("hh:mm"));
}

void InvoiceChatBubble::setupUI()
{
    setFrameStyle(QFrame::Box);
    setLineWidth(1);
    setContentsMargins(10, 8, 10, 8);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(4);
    
    m_messageLabel = new QLabel(m_message);
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    
    m_timestampLabel = new QLabel();
    m_timestampLabel->setStyleSheet("color: #888; font-size: 11px;");
    m_timestampLabel->setAlignment(Qt::AlignRight);
    
    layout->addWidget(m_messageLabel);
    layout->addWidget(m_timestampLabel);
    
    // Style based on type
    QString styleSheet;
    if (m_type == User) {
        styleSheet = "InvoiceChatBubble { background-color: #E3F2FD; border-color: #2196F3; border-radius: 12px; }";
        setMaximumWidth(400);
    } else if (m_type == Assistant) {
        styleSheet = "InvoiceChatBubble { background-color: #F5F5F5; border-color: #9E9E9E; border-radius: 12px; }";
        setMaximumWidth(500);
    } else { // System
        styleSheet = "InvoiceChatBubble { background-color: #FFF3E0; border-color: #FF9800; border-radius: 8px; }";
        m_messageLabel->setStyleSheet("font-style: italic; color: #666;");
    }
    
    setStyleSheet(styleSheet);
    setTimestamp(QDateTime::currentDateTime());
}

void InvoiceChatBubble::setupAnimations()
{
    m_fadeInAnimation = new QPropertyAnimation(this, "windowOpacity");
    m_fadeInAnimation->setDuration(300);
    m_fadeInAnimation->setStartValue(0.0);
    m_fadeInAnimation->setEndValue(1.0);
}

void InvoiceChatBubble::animateIn()
{
    m_fadeInAnimation->start();
}

void InvoiceChatBubble::startTypingAnimation()
{
    m_messageLabel->setText("● ● ●");
    // Simple animation could be added here
}

void InvoiceChatBubble::stopTypingAnimation()
{
    m_messageLabel->setText(m_message);
}

// InvoiceTypingIndicator implementation
InvoiceTypingIndicator::InvoiceTypingIndicator(QWidget *parent)
    : QWidget(parent), m_isActive(false), m_dotCount(0)
{
    setupUI();
    
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &InvoiceTypingIndicator::updateDots);
}

void InvoiceTypingIndicator::setupUI()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    m_label = new QLabel("AI Assistant is typing...");
    m_label->setStyleSheet("color: #666; font-style: italic; padding: 5px;");
    layout->addWidget(m_label);
    hide();
}

void InvoiceTypingIndicator::start()
{
    m_isActive = true;
    m_dotCount = 0;
    show();
    m_timer->start(500);
}

void InvoiceTypingIndicator::stop()
{
    m_isActive = false;
    m_timer->stop();
    hide();
}

void InvoiceTypingIndicator::updateDots()
{
    m_dotCount = (m_dotCount + 1) % 4;
    QString dots = QString(".").repeated(m_dotCount);
    m_label->setText(QString("AI Assistant is typing%1").arg(dots));
}

void InvoiceTypingIndicator::show()
{
    QWidget::show();
}

void InvoiceTypingIndicator::hide()
{
    QWidget::hide();
}

void InvoiceTypingIndicator::updateAnimation()
{
    if (m_isActive) {
        updateDots();
    }
}

// InvoiceAIAssistantDialog implementation
InvoiceAIAssistantDialog::InvoiceAIAssistantDialog(QWidget *parent)
    : QDialog(parent)
    , m_groqClient(nullptr)
    , m_databaseService(nullptr)
    , m_invoiceDbManager(nullptr)
    , m_currentInvoice(nullptr)
    , m_currentClient(nullptr)
    , m_autoScroll(true)
    , m_showTimestamps(true)
    , m_fontFamily("Segoe UI")
    , m_fontSize(10)
{
    setWindowTitle(tr("Invoice AI Assistant"));
    setMinimumSize(800, 600);
    resize(1000, 700);
    setModal(false);
    
    setupUI();
    setupAnimations();
    setupConnections();
    setupQuickActions();
    loadSettings();
    applyArchiFlowTheme();
}

InvoiceAIAssistantDialog::~InvoiceAIAssistantDialog()
{
    saveSettings();
}

void InvoiceAIAssistantDialog::setGroqClient(GroqClient *client)
{
    if (m_groqClient && m_groqClient->parent() == this) {
        m_groqClient->deleteLater();
    }
    
    m_groqClient = client;
    if (m_groqClient) {
        connect(m_groqClient, &GroqClient::messageReceived, this, &InvoiceAIAssistantDialog::onMessageReceived);
        connect(m_groqClient, &GroqClient::errorOccurred, this, &InvoiceAIAssistantDialog::onErrorOccurred);
        connect(m_groqClient, &GroqClient::requestStarted, this, &InvoiceAIAssistantDialog::onRequestStarted);
        connect(m_groqClient, &GroqClient::requestFinished, this, &InvoiceAIAssistantDialog::onRequestFinished);
        
        updateConnectionIndicator();
    }
}

void InvoiceAIAssistantDialog::setInvoiceDatabaseManager(InvoiceDatabaseManager *dbManager)
{
    m_invoiceDbManager = dbManager;
}

void InvoiceAIAssistantDialog::setInvoiceContext(const QJsonObject &context)
{
    m_invoiceContext = context;
}

void InvoiceAIAssistantDialog::setCurrentInvoice(const Invoice *invoice)
{
    m_currentInvoice = invoice;
    
    if (invoice) {
        QString welcomeMessage = tr("I'm ready to help you with Invoice #%1. "
                                   "You can ask me about invoice details, client information, "
                                   "calculations, or get suggestions for improvement.")
                                   .arg(invoice->invoiceNumber());
        addMessage(InvoiceChatBubble::System, welcomeMessage);
    }
}

void InvoiceAIAssistantDialog::setCurrentClient(const Client *client)
{
    m_currentClient = client;
}

void InvoiceAIAssistantDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Header
    m_headerFrame = new QFrame();
    m_headerFrame->setFrameStyle(QFrame::StyledPanel);
    m_headerFrame->setStyleSheet("background-color: #2C3E50; color: white;");
    m_headerFrame->setFixedHeight(50);
    
    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerFrame);
    
    m_titleLabel = new QLabel(tr("Invoice AI Assistant"));
    m_titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: white;");
    
    m_connectionIndicator = new QLabel("●");
    m_connectionIndicator->setStyleSheet("color: #E74C3C; font-size: 14px;");
    m_connectionIndicator->setToolTip(tr("Disconnected"));
    
    m_settingsButton = new QPushButton("⚙");
    m_settingsButton->setFixedSize(30, 30);
    m_settingsButton->setStyleSheet("QPushButton { border: none; color: white; font-size: 16px; } QPushButton:hover { background-color: #34495E; }");
    
    m_closeButton = new QPushButton("✕");
    m_closeButton->setFixedSize(30, 30);
    m_closeButton->setStyleSheet("QPushButton { border: none; color: white; font-size: 14px; } QPushButton:hover { background-color: #E74C3C; }");
    
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_connectionIndicator);
    headerLayout->addWidget(m_settingsButton);
    headerLayout->addWidget(m_closeButton);
    
    m_mainLayout->addWidget(m_headerFrame);
    
    // Main content with splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    
    // Chat area
    QWidget *chatWidget = new QWidget();
    QVBoxLayout *chatMainLayout = new QVBoxLayout(chatWidget);
    
    m_chatScrollArea = new QScrollArea();
    m_chatScrollArea->setWidgetResizable(true);
    m_chatScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_chatScrollArea->setStyleSheet("QScrollArea { border: none; }");
    
    m_chatWidget = new QWidget();
    m_chatLayout = new QVBoxLayout(m_chatWidget);
    m_chatLayout->addStretch();
    
    m_chatScrollArea->setWidget(m_chatWidget);
    chatMainLayout->addWidget(m_chatScrollArea);
    
    // Typing indicator
    m_typingIndicator = new InvoiceTypingIndicator();
    chatMainLayout->addWidget(m_typingIndicator);
    
    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    m_progressBar->setStyleSheet("QProgressBar { border: 2px solid grey; border-radius: 5px; text-align: center; }");
    chatMainLayout->addWidget(m_progressBar);
    
    // Input area
    m_inputFrame = new QFrame();
    m_inputFrame->setFrameStyle(QFrame::StyledPanel);
    m_inputFrame->setStyleSheet("background-color: #ECF0F1;");
    
    QHBoxLayout *inputLayout = new QHBoxLayout(m_inputFrame);
    
    m_attachButton = new QPushButton("📎");
    m_attachButton->setFixedSize(30, 30);
    m_attachButton->setToolTip(tr("Attach Document"));
    
    m_messageInput = new QLineEdit();
    m_messageInput->setPlaceholderText(tr("Ask me anything about your invoices..."));
    m_messageInput->setStyleSheet("QLineEdit { border: 1px solid #BDC3C7; border-radius: 15px; padding: 8px 12px; }");
    
    m_sendButton = new QPushButton("Send");
    m_sendButton->setFixedSize(60, 30);
    m_sendButton->setStyleSheet("QPushButton { background-color: #3498DB; color: white; border: none; border-radius: 15px; } QPushButton:hover { background-color: #2980B9; }");
    
    inputLayout->addWidget(m_attachButton);
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendButton);
    
    chatMainLayout->addWidget(m_inputFrame);
    
    // Sidebar
    m_sidebarFrame = new QFrame();
    m_sidebarFrame->setFrameStyle(QFrame::StyledPanel);
    m_sidebarFrame->setStyleSheet("background-color: #F8F9FA; border-left: 1px solid #DEE2E6;");
    m_sidebarFrame->setFixedWidth(250);
    
    m_sidebarLayout = new QVBoxLayout(m_sidebarFrame);
    
    m_quickActionsLabel = new QLabel(tr("Quick Actions"));
    m_quickActionsLabel->setStyleSheet("font-weight: bold; padding: 10px; background-color: #E9ECEF;");
    
    m_quickActionsList = new QListWidget();
    m_quickActionsList->setStyleSheet("QListWidget::item { padding: 8px; border-bottom: 1px solid #DEE2E6; } QListWidget::item:hover { background-color: #E3F2FD; }");
    
    m_sidebarLayout->addWidget(m_quickActionsLabel);
    m_sidebarLayout->addWidget(m_quickActionsList);
    m_sidebarLayout->addStretch();
    
    splitter->addWidget(chatWidget);
    splitter->addWidget(m_sidebarFrame);
    splitter->setSizes({700, 250});
    
    m_mainLayout->addWidget(splitter);
    
    // Initial welcome message
    addMessage(InvoiceChatBubble::System, tr("Welcome to the Invoice AI Assistant! I can help you with:") +
               "\n• Analyzing invoice data and calculations" +
               "\n• Generating invoice summaries and reports" +
               "\n• Client information and history" +
               "\n• Payment tracking and follow-ups" +
               "\n• Invoice optimization suggestions");
}

void InvoiceAIAssistantDialog::setupAnimations()
{
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(m_opacityEffect);
    
    m_fadeInAnimation = new QPropertyAnimation(m_opacityEffect, "opacity");
    m_fadeInAnimation->setDuration(300);
    m_fadeInAnimation->setStartValue(0.0);
    m_fadeInAnimation->setEndValue(1.0);
}

void InvoiceAIAssistantDialog::setupConnections()
{
    connect(m_sendButton, &QPushButton::clicked, this, &InvoiceAIAssistantDialog::sendMessage);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &InvoiceAIAssistantDialog::sendMessage);
    connect(m_attachButton, &QPushButton::clicked, this, &InvoiceAIAssistantDialog::attachDocument);
    connect(m_settingsButton, &QPushButton::clicked, this, &InvoiceAIAssistantDialog::showSettings);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::close);
    connect(m_quickActionsList, &QListWidget::itemClicked, this, &InvoiceAIAssistantDialog::onQuickActionClicked);
}

void InvoiceAIAssistantDialog::setupQuickActions()
{
    m_quickActions = {
        tr("Calculate invoice total"),
        tr("Verify line item calculations"),
        tr("Generate invoice summary"),
        tr("Check client payment history"),
        tr("Suggest payment terms"),
        tr("Format invoice for printing"),
        tr("Export invoice data"),
        tr("Create payment reminder"),
        tr("Analyze overdue invoices"),
        tr("Generate monthly report")
    };
    
    addInvoiceQuickActions();
}

void InvoiceAIAssistantDialog::addInvoiceQuickActions()
{
    m_quickActionsList->clear();
    for (const QString &action : m_quickActions) {
        m_quickActionsList->addItem(action);
    }
}

void InvoiceAIAssistantDialog::addMessage(InvoiceChatBubble::Type type, const QString &message)
{
    InvoiceChatBubble *bubble = new InvoiceChatBubble(type, message);
    
    QHBoxLayout *messageLayout = new QHBoxLayout();
    messageLayout->setContentsMargins(10, 5, 10, 5);
    
    if (type == InvoiceChatBubble::User) {
        messageLayout->addStretch();
        messageLayout->addWidget(bubble);
    } else {
        messageLayout->addWidget(bubble);
        messageLayout->addStretch();
    }
    
    // Insert before the stretch at the end
    m_chatLayout->insertLayout(m_chatLayout->count() - 1, messageLayout);
    
    bubble->animateIn();
    
    if (m_autoScroll) {
        QTimer::singleShot(100, this, &InvoiceAIAssistantDialog::scrollToBottom);
    }
    
    // Store in history
    ChatMessage chatMessage(
        type == InvoiceChatBubble::User ? "user" : 
        type == InvoiceChatBubble::Assistant ? "assistant" : "system",
        message
    );
    m_chatHistory.append(chatMessage);
}

void InvoiceAIAssistantDialog::scrollToBottom()
{
    QScrollBar *scrollBar = m_chatScrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void InvoiceAIAssistantDialog::sendMessage()
{
    QString message = m_messageInput->text().trimmed();
    if (message.isEmpty() || !m_groqClient) return;
    
    // Add user message
    addMessage(InvoiceChatBubble::User, message);
    
    // Clear input
    m_messageInput->clear();
    
    // Process message with context
    QString processedMessage = processUserMessage(message);
    
    // Send to AI
    QList<ChatMessage> context;
    
    // Add system context
    QString systemContext = tr("You are an AI assistant specialized in invoice management for ArchiFlow. ");
    if (m_currentInvoice) {
        systemContext += tr("Current invoice: #%1, Amount: %2, Status: %3. ")
                        .arg(m_currentInvoice->invoiceNumber())
                        .arg(m_currentInvoice->totalAmount())
                        .arg(m_currentInvoice->status());
    }
    if (m_currentClient) {
        systemContext += tr("Current client: %1. ").arg(m_currentClient->name());
    }
    systemContext += tr("Provide helpful, accurate, and professional advice about invoice management.");
    
    context.append(ChatMessage("system", systemContext));
    
    // Add recent chat history (last 10 messages)
    int historyCount = qMin(10, m_chatHistory.size());
    for (int i = m_chatHistory.size() - historyCount; i < m_chatHistory.size(); ++i) {
        if (m_chatHistory[i].role != "system") {
            context.append(m_chatHistory[i]);
        }
    }
    
    m_groqClient->sendChatCompletion(context);
}

QString InvoiceAIAssistantDialog::processUserMessage(const QString &message)
{
    QString processed = message;
    
    // Handle specific invoice commands
    if (message.contains("calculate", Qt::CaseInsensitive) && m_currentInvoice) {
        QString invoiceData = QString("Current invoice data: Total: %1, Tax: %2, Items: %3")
                            .arg(m_currentInvoice->totalAmount())
                            .arg(m_currentInvoice->taxAmount())
                            .arg(m_currentInvoice->items().size());
        processed += "\n\n" + invoiceData;
    }
    
    return processed;
}

void InvoiceAIAssistantDialog::onMessageReceived(const QString &message, const QString &messageId)
{
    Q_UNUSED(messageId)
    
    m_typingIndicator->stop();
    addMessage(InvoiceChatBubble::Assistant, message);
}

void InvoiceAIAssistantDialog::onErrorOccurred(const QString &error, int code)
{
    Q_UNUSED(code)
    
    m_typingIndicator->stop();
    addMessage(InvoiceChatBubble::System, tr("Error: %1").arg(error));
}

void InvoiceAIAssistantDialog::onRequestStarted()
{
    m_typingIndicator->start();
}

void InvoiceAIAssistantDialog::onRequestFinished()
{
    m_typingIndicator->stop();
}

void InvoiceAIAssistantDialog::onQuickActionClicked()
{
    QListWidgetItem *item = m_quickActionsList->currentItem();
    if (!item) return;
    
    QString action = item->text();
    m_messageInput->setText(action);
    sendMessage();
}

void InvoiceAIAssistantDialog::clearChat()
{
    // Clear all messages except system messages
    while (m_chatLayout->count() > 1) {
        QLayoutItem *item = m_chatLayout->takeAt(0);
        if (item) {
            delete item->widget();
            delete item;
        }
    }
    
    m_chatHistory.clear();
    
    // Add welcome message back
    addMessage(InvoiceChatBubble::System, tr("Chat cleared. How can I help you with your invoices?"));
}

void InvoiceAIAssistantDialog::exportChat()
{
    QString fileName = QFileDialog::getSaveFileName(this, 
        tr("Export Chat"), 
        QString("invoice_chat_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        tr("Text Files (*.txt)"));
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << tr("Invoice AI Assistant Chat Export") << "\n";
        out << tr("Date: %1").arg(QDateTime::currentDateTime().toString()) << "\n\n";
        
        for (const ChatMessage &msg : m_chatHistory) {
            out << QString("[%1] %2: %3\n")
                  .arg(QDateTime::fromMSecsSinceEpoch(msg.timestamp).toString("hh:mm:ss"))
                  .arg(msg.role.toUpper())
                  .arg(msg.content);
        }
    }
}

void InvoiceAIAssistantDialog::showSettings()
{
    QMessageBox::information(this, tr("Settings"), tr("Settings dialog will be implemented in a future version."));
}

void InvoiceAIAssistantDialog::attachDocument()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Attach Document"),
        "",
        tr("All Files (*.*);;PDF Files (*.pdf);;Text Files (*.txt);;Excel Files (*.xlsx *.xls)"));
    
    if (!fileName.isEmpty()) {
        onDocumentSelected(fileName);
    }
}

void InvoiceAIAssistantDialog::onDocumentSelected(const QString &filePath)
{
    analyzeDocument(filePath);
}

void InvoiceAIAssistantDialog::analyzeDocument(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString message = tr("I've attached a document: %1. Please analyze it and provide insights.")
                     .arg(fileInfo.fileName());
    
    // For now, just add the message - document analysis would require more complex implementation
    addMessage(InvoiceChatBubble::User, message);
    addMessage(InvoiceChatBubble::System, tr("Document analysis feature will be implemented in a future version."));
}

void InvoiceAIAssistantDialog::updateConnectionIndicator()
{
    if (m_groqClient) {
        m_connectionIndicator->setStyleSheet("color: #27AE60; font-size: 14px;");
        m_connectionIndicator->setToolTip(tr("Connected"));
    } else {
        m_connectionIndicator->setStyleSheet("color: #E74C3C; font-size: 14px;");
        m_connectionIndicator->setToolTip(tr("Disconnected"));
    }
}

void InvoiceAIAssistantDialog::applyArchiFlowTheme()
{
    setStyleSheet(
        "InvoiceAIAssistantDialog {"
        "    background-color: #FFFFFF;"
        "    font-family: 'Segoe UI', Arial, sans-serif;"
        "}"
        "QScrollArea {"
        "    background-color: #FFFFFF;"
        "    border: none;"
        "}"
    );
}

void InvoiceAIAssistantDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    animateDialogEntry();
    m_messageInput->setFocus();
}

void InvoiceAIAssistantDialog::animateDialogEntry()
{
    m_fadeInAnimation->start();
}

void InvoiceAIAssistantDialog::saveSettings()
{
    // Settings would be saved to QSettings
}

void InvoiceAIAssistantDialog::loadSettings()
{
    // Settings would be loaded from QSettings
}

void InvoiceAIAssistantDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void InvoiceAIAssistantDialog::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        if (!urls.isEmpty()) {
            QString filePath = urls.first().toLocalFile();
            onDocumentSelected(filePath);
        }
    }
}

void InvoiceAIAssistantDialog::onConnectionStatusChanged(bool connected)
{
    Q_UNUSED(connected)
    updateConnectionIndicator();
}

void InvoiceAIAssistantDialog::onTypingStarted()
{
    if (m_typingIndicator) {
        m_typingIndicator->start();
    }
}

void InvoiceAIAssistantDialog::onTypingFinished()
{
    if (m_typingIndicator) {
        m_typingIndicator->stop();
    }
}

void InvoiceAIAssistantDialog::processDatabaseCommand(const QString &command)
{
    // Process database commands - placeholder implementation
    addMessage(InvoiceChatBubble::System, tr("Processing database command: %1").arg(command));
}

void InvoiceAIAssistantDialog::handleDatabaseQuery(const QString &query)
{
    // Handle database queries - placeholder implementation
    if (m_invoiceDbManager) {
        addMessage(InvoiceChatBubble::System, tr("Executing database query: %1").arg(query));
        // Actual database query implementation would go here
    }
}

void InvoiceAIAssistantDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (m_autoScroll) {
        QTimer::singleShot(100, this, &InvoiceAIAssistantDialog::scrollToBottom);
    }
}

void InvoiceAIAssistantDialog::hideEvent(QHideEvent *event)
{
    QDialog::hideEvent(event);
    if (m_typingIndicator) {
        m_typingIndicator->stop();
    }
}
