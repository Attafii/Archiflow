#include "chatbotdialog.h"
#include <QApplication>
#include <QScreen>
#include <QKeyEvent>
#include <QScrollBar>
#include <QMovie>

// Styling constants matching the main application
const QString ChatbotDialog::DIALOG_STYLE = R"(
    QDialog {
        background-color: #3D485A;
        border: 2px solid #E3C6B0;
        border-radius: 10px;
    }
    QScrollArea {
        background-color: #3D485A;
        border: none;
        border-radius: 5px;
    }
    QScrollArea QWidget {
        background-color: #3D485A;
    }
    QScrollBar:vertical {
        background-color: #2A3441;
        width: 12px;
        border-radius: 6px;
    }
    QScrollBar::handle:vertical {
        background-color: #E3C6B0;
        border-radius: 6px;
        min-height: 20px;
    }
    QScrollBar::handle:vertical:hover {
        background-color: #C4A491;
    }
)";

const QString ChatbotDialog::MESSAGE_USER_STYLE = R"(
    QFrame {
        background-color: #E3C6B0;
        border-radius: 15px;
        padding: 10px;
        margin: 5px;
        max-width: 300px;
    }
    QLabel {
        color: #3D485A;
        font-family: 'Poppins', Arial, sans-serif;
        font-size: 14px;
        background: transparent;
        border: none;
        padding: 8px 12px;
    }
)";

const QString ChatbotDialog::MESSAGE_BOT_STYLE = R"(
    QFrame {
        background-color: #2A3441;
        border: 1px solid #E3C6B0;
        border-radius: 15px;
        padding: 10px;
        margin: 5px;
        max-width: 350px;
    }
    QLabel {
        color: #E3C6B0;
        font-family: 'Poppins', Arial, sans-serif;
        font-size: 14px;
        background: transparent;
        border: none;
        padding: 8px 12px;
    }
)";

const QString ChatbotDialog::INPUT_STYLE = R"(
    QLineEdit {
        background-color: white;
        border: 2px solid #E3C6B0;
        border-radius: 20px;
        padding: 12px 16px;
        font-family: 'Poppins', Arial, sans-serif;
        font-size: 14px;
        color: black;
    }
    QLineEdit:focus {
        border: 2px solid #C4A491;
        outline: none;
    }
)";

const QString ChatbotDialog::BUTTON_STYLE = R"(
    QPushButton {
        background-color: #E3C6B0;
        color: #3D485A;
        border: none;
        border-radius: 20px;
        padding: 12px 20px;
        font-family: 'Poppins', Arial, sans-serif;
        font-size: 14px;
        font-weight: bold;
        min-width: 80px;
    }
    QPushButton:hover {
        background-color: #C4A491;
    }
    QPushButton:pressed {
        background-color: #A68B78;
    }
    QPushButton:disabled {
        background-color: #8A8A8A;
        color: #FFFFFF;
    }
)";

ChatbotDialog::ChatbotDialog(ChatbotManager* chatbotManager, QWidget *parent)
    : QDialog(parent)
    , m_chatbotManager(chatbotManager)
    , m_typingTimer(new QTimer(this))
{
    setupUI();
    
    // Connect chatbot signals
    connect(m_chatbotManager, &ChatbotManager::responseReady,
            this, &ChatbotDialog::onResponseReady);
    connect(m_chatbotManager, &ChatbotManager::errorOccurred,
            this, &ChatbotDialog::onErrorOccurred);
    
    // Add welcome message
    addMessage("Hello! I'm your AI assistant for contract management. How can I help you today?\n\n"
               "I can help you with:\n"
               "• Contract updates (e.g., 'change contract ID:120 client name to NewName')\n"
               "• Contract searches (e.g., 'show me all active contracts')\n"
               "• Contract creation (e.g., 'create a contract for ABC Corp')\n"
               "• Statistics (e.g., 'get contract statistics')\n"
               "• And much more!", false);
}

ChatbotDialog::~ChatbotDialog()
{
}

void ChatbotDialog::setupUI()
{
    setWindowTitle("AI Contract Assistant");
    setModal(false);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    resize(450, 600);
    
    // Center the dialog on the parent window
    if (parentWidget()) {
        QRect parentRect = parentWidget()->geometry();
        move(parentRect.center() - rect().center());
    }
    
    // Apply main dialog styling
    setStyleSheet(DIALOG_STYLE);
    
    // Main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(10);
    
    // Title
    QLabel *titleLabel = new QLabel("🤖 AI Contract Assistant");
    titleLabel->setStyleSheet(R"(
        QLabel {
            color: #E3C6B0;
            font-family: 'Poppins', Arial, sans-serif;
            font-size: 18px;
            font-weight: bold;
            padding: 10px;
            background: transparent;
            border: none;
        }
    )");
    titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(titleLabel);
    
    // Chat area
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_chatContainer = new QWidget();
    m_chatLayout = new QVBoxLayout(m_chatContainer);
    m_chatLayout->setContentsMargins(10, 10, 10, 10);
    m_chatLayout->setSpacing(8);
    m_chatLayout->addStretch(); // Push messages to bottom initially
    
    m_scrollArea->setWidget(m_chatContainer);
    m_mainLayout->addWidget(m_scrollArea, 1);
    
    // Typing indicator
    m_typingIndicator = new QLabel("🤖 AI is typing...");
    m_typingIndicator->setStyleSheet(R"(
        QLabel {
            color: #C4A491;
            font-family: 'Poppins', Arial, sans-serif;
            font-size: 12px;
            font-style: italic;
            padding: 5px 15px;
            background: transparent;
            border: none;
        }
    )");
    m_typingIndicator->hide();
    m_mainLayout->addWidget(m_typingIndicator);
    
    // Input area
    m_inputLayout = new QHBoxLayout();
    m_inputLayout->setSpacing(10);
    
    m_inputField = new QLineEdit();
    m_inputField->setStyleSheet(INPUT_STYLE);
    m_inputField->setPlaceholderText("Type your message here...");
    
    m_sendButton = new QPushButton("Send");
    m_sendButton->setStyleSheet(BUTTON_STYLE);
    
    m_inputLayout->addWidget(m_inputField, 1);
    m_inputLayout->addWidget(m_sendButton);
    
    m_mainLayout->addLayout(m_inputLayout);
    
    // Connect signals
    connect(m_sendButton, &QPushButton::clicked, this, &ChatbotDialog::sendMessage);
    connect(m_inputField, &QLineEdit::returnPressed, this, &ChatbotDialog::sendMessage);
    
    // Focus on input field
    m_inputField->setFocus();
}

void ChatbotDialog::addMessage(const QString &message, bool isUser)
{
    // Remove the stretch if it exists
    if (m_chatLayout->count() > 0) {
        QLayoutItem *item = m_chatLayout->itemAt(m_chatLayout->count() - 1);
        if (item && item->spacerItem()) {
            m_chatLayout->removeItem(item);
            delete item;
        }
    }
    
    // Create message container
    QFrame *messageFrame = new QFrame();
    messageFrame->setFrameStyle(QFrame::NoFrame);
    
    QHBoxLayout *messageLayout = new QHBoxLayout(messageFrame);
    messageLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create message bubble
    QFrame *bubble = new QFrame();
    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(0, 0, 0, 0);
    
    // Message text
    QLabel *messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    
    bubbleLayout->addWidget(messageLabel);
    
    if (isUser) {
        // User message - align right
        bubble->setStyleSheet(MESSAGE_USER_STYLE);
        messageLayout->addStretch();
        messageLayout->addWidget(bubble);
        bubble->setMaximumWidth(300);
    } else {
        // Bot message - align left
        bubble->setStyleSheet(MESSAGE_BOT_STYLE);
        messageLayout->addWidget(bubble);
        messageLayout->addStretch();
        bubble->setMaximumWidth(350);
    }
    
    m_chatLayout->addWidget(messageFrame);
    
    // Add stretch at the end
    m_chatLayout->addStretch();
    
    // Scroll to bottom
    QTimer::singleShot(50, this, &ChatbotDialog::scrollToBottom);
}

void ChatbotDialog::scrollToBottom()
{
    QScrollBar *scrollBar = m_scrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void ChatbotDialog::sendMessage()
{
    QString message = m_inputField->text().trimmed();
    if (message.isEmpty()) {
        return;
    }
    
    // Add user message to chat
    addMessage(message, true);
    
    // Clear input field
    m_inputField->clear();
    
    // Disable input while processing
    m_inputField->setEnabled(false);
    m_sendButton->setEnabled(false);
    
    // Show typing indicator
    showTypingIndicator();
    
    // Send message to chatbot
    m_chatbotManager->processUserRequest(message, m_selectedContractId);
}

void ChatbotDialog::onResponseReady(const QString &response)
{
    hideTypingIndicator();
    addMessage(response, false);
    
    // Re-enable input
    m_inputField->setEnabled(true);
    m_sendButton->setEnabled(true);
    m_inputField->setFocus();
}

void ChatbotDialog::onErrorOccurred(const QString &error)
{
    hideTypingIndicator();
    addMessage("❌ " + error, false);
    
    // Re-enable input
    m_inputField->setEnabled(true);
    m_sendButton->setEnabled(true);
    m_inputField->setFocus();
}

void ChatbotDialog::showTypingIndicator()
{
    m_typingIndicator->show();
    scrollToBottom();
}

void ChatbotDialog::hideTypingIndicator()
{
    m_typingIndicator->hide();
}

void ChatbotDialog::setSelectedContractId(const QString &contractId)
{
    m_selectedContractId = contractId;
    
    if (!contractId.isEmpty()) {
        addMessage(QString("📋 Selected contract: %1").arg(contractId), false);
    }
}

void ChatbotDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    } else {
        QDialog::keyPressEvent(event);
    }
}
