#include "clientaiassistant.h"
#include "../../utils/environmentloader.h"
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QGroupBox>
#include <QDateTime>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QScrollBar>

ClientAIAssistant::ClientAIAssistant(ClientContact *client, QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
    , m_chatWidget(nullptr)
    , m_chatLayout(nullptr)
    , m_chatArea(nullptr)
    , m_chatContent(nullptr)
    , m_chatContentLayout(nullptr)
    , m_messageInput(nullptr)
    , m_sendBtn(nullptr)
    , m_clearBtn(nullptr)
    , m_insightsWidget(nullptr)
    , m_insightsLayout(nullptr)
    , m_insightsDisplay(nullptr)
    , m_generateInsightsBtn(nullptr)
    , m_copyInsightsBtn(nullptr)
    , m_networkManager(nullptr)
    , m_typingTimer(nullptr)
    , m_client(client)
{
    loadEnvironmentVariables();
    setupUI();
    setupConnections();
    
    setWindowTitle(client ? 
        tr("AI Assistant - %1").arg(client->name()) : 
        tr("AI Assistant - Client Management"));
    
    resize(900, 700);
    setModal(false);
}

void ClientAIAssistant::setClient(ClientContact *client)
{
    m_client = client;
    setWindowTitle(client ? 
        tr("AI Assistant - %1").arg(client->name()) : 
        tr("AI Assistant - Client Management"));
    
    // Clear previous chat when switching clients
    onClearChat();
}

void ClientAIAssistant::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Header
    QLabel *headerLabel = new QLabel("🤖 AI Assistant - Client Management");
    headerLabel->setStyleSheet(
        "QLabel { "
        "font-size: 18px; "
        "font-weight: bold; "
        "color: #2c3e50; "
        "padding: 15px; "
        "background-color: #ecf0f1; "
        "border-radius: 8px; "
        "margin-bottom: 10px; "
        "}"
    );
    headerLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(headerLabel);
    
    // Main splitter
    m_splitter = new QSplitter(Qt::Horizontal);
    
    // === Chat Section ===
    setupChatSection();
    
    // === Insights Section ===
    setupInsightsSection();
    
    m_splitter->addWidget(m_chatWidget);
    m_splitter->addWidget(m_insightsWidget);
    m_splitter->setSizes({400, 400});
    
    m_mainLayout->addWidget(m_splitter);
    
    // Status
    QLabel *statusLabel = new QLabel();
    if (m_groqApiKey.isEmpty()) {
        statusLabel->setText("⚠️ Configure GROQ_API_KEY in .env file for AI functionality");
        statusLabel->setStyleSheet("color: #e74c3c; font-weight: bold; padding: 5px;");
    } else {
        statusLabel->setText("✅ AI Assistant ready - Powered by Groq");
        statusLabel->setStyleSheet("color: #27ae60; font-weight: bold; padding: 5px;");
    }
    m_mainLayout->addWidget(statusLabel);
    
    setLayout(m_mainLayout);
}

void ClientAIAssistant::setupChatSection()
{
    m_chatWidget = new QWidget();
    m_chatLayout = new QVBoxLayout(m_chatWidget);
    
    // Chat header
    QLabel *chatHeader = new QLabel("💬 AI Chatbot");
    chatHeader->setStyleSheet(
        "QLabel { "
        "font-size: 14px; "
        "font-weight: bold; "
        "color: #34495e; "
        "padding: 8px; "
        "background-color: #f8f9fa; "
        "border-radius: 5px; "
        "}"
    );
    m_chatLayout->addWidget(chatHeader);
    
    // Chat area
    m_chatArea = new QScrollArea();
    m_chatContent = new QWidget();
    m_chatContentLayout = new QVBoxLayout(m_chatContent);
    m_chatContentLayout->setAlignment(Qt::AlignTop);
    m_chatContentLayout->setSpacing(10);
    
    m_chatArea->setWidget(m_chatContent);
    m_chatArea->setWidgetResizable(true);
    m_chatArea->setStyleSheet(
        "QScrollArea { "
        "border: 1px solid #dee2e6; "
        "border-radius: 5px; "
        "background-color: white; "
        "}"
    );
    m_chatLayout->addWidget(m_chatArea);
    
    // Input area
    QHBoxLayout *inputLayout = new QHBoxLayout();
    
    m_messageInput = new QLineEdit();
    m_messageInput->setPlaceholderText("Ask about client management, insights, or strategies...");
    m_messageInput->setStyleSheet(
        "QLineEdit { "
        "padding: 10px; "
        "border: 2px solid #dee2e6; "
        "border-radius: 5px; "
        "font-size: 14px; "
        "} "
        "QLineEdit:focus { border-color: #3498db; }"
    );
    
    m_sendBtn = new QPushButton("Send");
    m_sendBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #3498db; "
        "color: white; "
        "border: none; "
        "padding: 10px 20px; "
        "border-radius: 5px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { background-color: #2980b9; } "
        "QPushButton:disabled { background-color: #bdc3c7; }"
    );
    
    m_clearBtn = new QPushButton("Clear");
    m_clearBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #95a5a6; "
        "color: white; "
        "border: none; "
        "padding: 10px 15px; "
        "border-radius: 5px; "
        "} "
        "QPushButton:hover { background-color: #7f8c8d; }"
    );
    
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendBtn);
    inputLayout->addWidget(m_clearBtn);
    
    m_chatLayout->addLayout(inputLayout);
    
    // Add welcome message
    addMessageToChat("Hello! I'm your AI assistant for client management. I can help you with:\n\n"
                    "• Client relationship strategies\n"
                    "• Communication best practices\n"
                    "• Project insights and analysis\n"
                    "• Business development advice\n\n"
                    "What would you like to know?", false);
}

void ClientAIAssistant::setupInsightsSection()
{
    m_insightsWidget = new QWidget();
    m_insightsLayout = new QVBoxLayout(m_insightsWidget);
    
    // Insights header
    QLabel *insightsHeader = new QLabel("🔍 AI Insights");
    insightsHeader->setStyleSheet(
        "QLabel { "
        "font-size: 14px; "
        "font-weight: bold; "
        "color: #34495e; "
        "padding: 8px; "
        "background-color: #f8f9fa; "
        "border-radius: 5px; "
        "}"
    );
    m_insightsLayout->addWidget(insightsHeader);
    
    // Insights display
    m_insightsDisplay = new QTextEdit();
    m_insightsDisplay->setReadOnly(true);
    m_insightsDisplay->setPlaceholderText("Click 'Generate Insights' to get AI-powered analysis of your client data...");
    m_insightsDisplay->setStyleSheet(
        "QTextEdit { "
        "border: 1px solid #dee2e6; "
        "border-radius: 5px; "
        "background-color: #f8f9fa; "
        "padding: 15px; "
        "font-family: 'Segoe UI', sans-serif; "
        "line-height: 1.4; "
        "}"
    );
    m_insightsLayout->addWidget(m_insightsDisplay);
    
    // Insights buttons
    QHBoxLayout *insightsButtonLayout = new QHBoxLayout();
    
    m_generateInsightsBtn = new QPushButton("🔮 Generate Insights");
    m_generateInsightsBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #9b59b6; "
        "color: white; "
        "border: none; "
        "padding: 10px 20px; "
        "border-radius: 5px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { background-color: #8e44ad; } "
        "QPushButton:disabled { background-color: #bdc3c7; }"
    );
    
    m_copyInsightsBtn = new QPushButton("📋 Copy");
    m_copyInsightsBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #2ecc71; "
        "color: white; "
        "border: none; "
        "padding: 10px 15px; "
        "border-radius: 5px; "
        "} "
        "QPushButton:hover { background-color: #27ae60; }"
    );
    
    insightsButtonLayout->addWidget(m_generateInsightsBtn);
    insightsButtonLayout->addWidget(m_copyInsightsBtn);
    insightsButtonLayout->addStretch();
    
    m_insightsLayout->addLayout(insightsButtonLayout);
}

void ClientAIAssistant::setupConnections()
{
    // Network manager
    m_networkManager = new QNetworkAccessManager(this);
    
    // Typing timer
    m_typingTimer = new QTimer(this);
    m_typingTimer->setSingleShot(true);
    
    // Chat connections
    connect(m_sendBtn, &QPushButton::clicked, this, &ClientAIAssistant::onSendMessage);
    connect(m_clearBtn, &QPushButton::clicked, this, &ClientAIAssistant::onClearChat);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &ClientAIAssistant::onSendMessage);
    
    // Insights connections
    connect(m_generateInsightsBtn, &QPushButton::clicked, this, &ClientAIAssistant::onGenerateInsights);
    connect(m_copyInsightsBtn, &QPushButton::clicked, this, &ClientAIAssistant::onCopyResponse);
    
    // Enable/disable based on API key
    bool hasApiKey = !m_groqApiKey.isEmpty();
    m_sendBtn->setEnabled(hasApiKey);
    m_generateInsightsBtn->setEnabled(hasApiKey);
}

void ClientAIAssistant::loadEnvironmentVariables()
{
    EnvironmentLoader::loadFromFile(".env");
    m_groqApiKey = EnvironmentLoader::getEnv("GROQ_API_KEY");
    
    if (m_groqApiKey.isEmpty()) {
        qWarning() << "ClientAIAssistant: No Groq API key found in environment";
    } else {
        qDebug() << "ClientAIAssistant: Initialized with Groq API key";
    }
}

void ClientAIAssistant::onSendMessage()
{
    QString message = m_messageInput->text().trimmed();
    if (message.isEmpty() || m_groqApiKey.isEmpty()) return;
    
    // Add user message to chat
    addMessageToChat(message, true);
    m_messageInput->clear();
    
    // Show typing indicator
    showTypingIndicator();
    
    // Prepare context and send to AI
    QString context = generateClientContext();
    QString fullPrompt = QString("Context: %1\n\nUser Question: %2").arg(context, message);
    
    sendToGroqAPI(fullPrompt);
}

void ClientAIAssistant::onClearChat()
{
    // Clear chat content
    QLayoutItem *item;
    while ((item = m_chatContentLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    m_chatHistory.clear();
    
    // Add welcome message back
    addMessageToChat("Chat cleared. How can I assist you with client management?", false);
}

void ClientAIAssistant::onGenerateInsights()
{
    if (m_groqApiKey.isEmpty()) return;
    
    m_generateInsightsBtn->setEnabled(false);
    m_generateInsightsBtn->setText("🔄 Generating...");
    
    QString insightPrompt = generateInsightPrompt();
    sendToGroqAPI(insightPrompt, "insights");
}

void ClientAIAssistant::onHandleAIResponse()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    // Hide typing indicator
    hideTypingIndicator();
    
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "ClientAIAssistant: API error:" << reply->errorString();
        addMessageToChat("Sorry, I encountered an error. Please try again.", false);
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull()) {
        addMessageToChat("Sorry, I received an invalid response. Please try again.", false);
        return;
    }
    
    QJsonObject response = doc.object();
    processAIResponse(response);
}

void ClientAIAssistant::onCopyResponse()
{
    QString text = m_insightsDisplay->toPlainText();
    if (!text.isEmpty()) {
        QApplication::clipboard()->setText(text);
        
        QString originalText = m_copyInsightsBtn->text();
        m_copyInsightsBtn->setText("✅ Copied!");
        QTimer::singleShot(2000, [this, originalText]() {
            if (m_copyInsightsBtn) m_copyInsightsBtn->setText(originalText);
        });
    }
}

void ClientAIAssistant::addMessageToChat(const QString &message, bool isUser)
{
    QFrame *messageFrame = createMessageFrame(message, isUser);
    m_chatContentLayout->addWidget(messageFrame);
    
    // Add to history
    QString historyEntry = QString("[%1] %2: %3")
                          .arg(QDateTime::currentDateTime().toString("hh:mm"))
                          .arg(isUser ? "User" : "AI")
                          .arg(message);
    m_chatHistory.append(historyEntry);
    
    // Scroll to bottom
    QTimer::singleShot(100, this, &ClientAIAssistant::scrollToBottom);
}

void ClientAIAssistant::sendToGroqAPI(const QString &prompt, const QString &context)
{
    QUrl url("https://api.groq.com/openai/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_groqApiKey).toUtf8());
    
    QJsonObject message;
    message["role"] = "user";
    message["content"] = prompt;
    
    QJsonArray messages;
    messages.append(message);
    
    QJsonObject requestBody;
    requestBody["model"] = "mixtral-8x7b-32768";
    requestBody["messages"] = messages;
    requestBody["max_tokens"] = 1024;
    requestBody["temperature"] = 0.7;
    
    QJsonDocument doc(requestBody);
    QByteArray data = doc.toJson();
    
    QNetworkReply *reply = m_networkManager->post(request, data);
    reply->setProperty("context", context);
    connect(reply, &QNetworkReply::finished, this, &ClientAIAssistant::onHandleAIResponse);
}

void ClientAIAssistant::processAIResponse(const QJsonObject &response)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    QString context = reply ? reply->property("context").toString() : "";
    
    QJsonArray choices = response["choices"].toArray();
    if (choices.isEmpty()) {
        addMessageToChat("Sorry, I couldn't generate a response. Please try again.", false);
        return;
    }
    
    QJsonObject choice = choices[0].toObject();
    QJsonObject message = choice["message"].toObject();
    QString content = message["content"].toString();
    
    if (context == "insights") {
        // Update insights display
        m_insightsDisplay->setPlainText(content);
        m_generateInsightsBtn->setEnabled(true);
        m_generateInsightsBtn->setText("🔮 Generate Insights");
    } else {
        // Add to chat
        addMessageToChat(content, false);
    }
}

QString ClientAIAssistant::generateClientContext()
{
    QString context = "You are an AI assistant for ArchiFlow, a client management system for architecture and construction projects. ";
      if (m_client) {
        context += QString("Current client details:\n");
        context += QString("- Name: %1\n").arg(m_client->name());
        context += QString("- Company: %1\n").arg(m_client->companyName());
        context += QString("- Email: %1\n").arg(m_client->email());
        context += QString("- Phone: %1\n").arg(m_client->phoneNumber());
        context += QString("- Address: %1, %2, %3 %4\n")
                   .arg(m_client->addressStreet())
                   .arg(m_client->addressCity())
                   .arg(m_client->addressState())
                   .arg(m_client->addressZipcode());
        context += QString("- Notes: %1\n").arg(m_client->notes());
    } else {
        context += "No specific client selected. Provide general client management advice.";
    }
    
    context += "\nPlease provide helpful, professional advice about client relationships, project management, and business development in the architecture/construction industry.";
    
    return context;
}

QString ClientAIAssistant::generateInsightPrompt()
{
    QString prompt = "As an AI business analyst for ArchiFlow, provide detailed insights and recommendations for ";
    
    if (m_client) {
        prompt += QString("client '%1' based on the following information:\n\n").arg(m_client->name());        prompt += QString("Client: %1\n").arg(m_client->name());
        prompt += QString("Company: %1\n").arg(m_client->companyName());
        prompt += QString("Industry: Architecture/Construction\n");
        prompt += QString("Location: %1, %2\n").arg(m_client->addressCity(), m_client->addressState());
        prompt += QString("Contact: %1 | %2\n").arg(m_client->email(), m_client->phoneNumber());
        if (!m_client->notes().isEmpty()) {
            prompt += QString("Additional Notes: %1\n").arg(m_client->notes());
        }
        
        prompt += "\nPlease analyze and provide insights on:\n";
        prompt += "1. Client Relationship Opportunities\n";
        prompt += "2. Potential Project Types and Services\n";
        prompt += "3. Communication Strategy Recommendations\n";
        prompt += "4. Market Analysis for their Location\n";
        prompt += "5. Risk Assessment and Mitigation\n";
        prompt += "6. Growth and Expansion Opportunities\n";
        prompt += "7. Competitive Advantages to Leverage\n";
        prompt += "8. Key Performance Indicators to Track\n\n";
        prompt += "Format the response with clear headings and actionable recommendations.";
    } else {
        prompt += "general client management in the architecture and construction industry. ";
        prompt += "Provide insights on best practices, common challenges, and strategic opportunities.";
    }
    
    return prompt;
}

QFrame* ClientAIAssistant::createMessageFrame(const QString &message, bool isUser)
{
    QFrame *frame = new QFrame();
    frame->setFrameStyle(QFrame::StyledPanel);
    
    QString frameStyle;
    if (isUser) {
        frameStyle = "QFrame { "
                    "background-color: #3498db; "
                    "color: white; "
                    "border-radius: 10px; "
                    "margin: 5px 50px 5px 5px; "
                    "padding: 10px; "
                    "}";
    } else {
        frameStyle = "QFrame { "
                    "background-color: #ecf0f1; "
                    "color: #2c3e50; "
                    "border-radius: 10px; "
                    "margin: 5px 5px 5px 50px; "
                    "padding: 10px; "
                    "}";
    }
    frame->setStyleSheet(frameStyle);
    
    QVBoxLayout *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(15, 10, 15, 10);
    
    QLabel *messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setStyleSheet("QLabel { background: transparent; }");
    
    QLabel *timeLabel = new QLabel(QDateTime::currentDateTime().toString("hh:mm"));
    timeLabel->setStyleSheet("QLabel { font-size: 10px; color: #7f8c8d; background: transparent; }");
    timeLabel->setAlignment(isUser ? Qt::AlignRight : Qt::AlignLeft);
    
    layout->addWidget(messageLabel);
    layout->addWidget(timeLabel);
    
    return frame;
}

void ClientAIAssistant::scrollToBottom()
{
    QScrollBar *scrollBar = m_chatArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void ClientAIAssistant::showTypingIndicator()
{
    addMessageToChat("🤖 AI is thinking...", false);
}

void ClientAIAssistant::hideTypingIndicator()
{
    // Remove the last "thinking" message
    QLayoutItem *item = m_chatContentLayout->takeAt(m_chatContentLayout->count() - 1);
    if (item && item->widget()) {
        delete item->widget();
        delete item;
    }
}
