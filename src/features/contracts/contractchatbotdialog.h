#ifndef CONTRACTCHATBOTDIALOG_H
#define CONTRACTCHATBOTDIALOG_H

#include "ui/basedialog.h"
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSplitter>
#include <QComboBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QTimer>
#include <QJsonObject>

class Contract;
class IContractChatbot;
class ContractDatabaseManager;

/**
 * @brief Dialog for AI-powered contract analysis and assistance
 * 
 * This dialog provides an interactive chatbot interface for users to:
 * - Ask questions about contracts
 * - Get AI-powered analysis and recommendations
 * - Compare multiple contracts
 * - Identify risks and improvement opportunities
 */
class ContractChatbotDialog : public BaseDialog
{
    Q_OBJECT

public:
    explicit ContractChatbotDialog(QWidget *parent = nullptr);
    ~ContractChatbotDialog();

    // Configuration
    void setChatbot(IContractChatbot *chatbot);
    void setDatabaseManager(ContractDatabaseManager *dbManager);
    void setCurrentContract(const Contract *contract);
    void setContracts(const QList<Contract*> &contracts);

    // Chat operations
    void startNewConversation();
    void loadConversationHistory(const QString &sessionId);
    void saveConversationHistory();

public slots:
    void sendMessage();
    void clearChat();
    void exportChat();
    void analyzeCurrentContract();
    void compareSelectedContracts();

signals:
    void analysisRequested(const QString &contractId);
    void contractComparisonRequested(const QStringList &contractIds);
    void chatExported(const QString &filePath);
    void contractAnalysisRequested(const QString &contractId);
    void contractCreationRequested(const QString &templateData);
    void errorOccurred(const QString &error);

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onSendClicked();
    void onMessageChanged();
    void onAnalysisModeChanged();
    void onSuggestionClicked();
    void onTypingTimerTimeout();
    void onChatModeChanged();
    void onSettingsClicked();

private:
    void setupUi();
    void setupChatArea();
    void setupInputArea();
    void setupControlPanel();
    void setupConnections();
    void applyArchiFlowStyling();

    // Chat operations
    void addMessage(const QString &sender, const QString &message, bool isUser = true);
    void addSystemMessage(const QString &message);
    void showTypingIndicator();
    void hideTypingIndicator();
    void processUserMessage(const QString &message);
    void displayBotResponse(const QString &response);
    void updateSuggestions();

    // UI helpers
    QWidget* createMessageWidget(const QString &sender, const QString &message, bool isUser);
    QWidget* createSuggestionWidget(const QString &suggestion);
    void scrollToBottom();
    void enableInput(bool enabled);
    void updateAnalysisOptions();
    void loadQuickActions();

    // Analysis operations
    void performContractAnalysis();
    void performContractComparison();
    void performRiskAssessment();
    void generateRecommendations();
    void extractKeyTerms();

    // Data formatting
    QString formatAnalysisResult(const QJsonObject &result);
    QString formatContractSummary(const Contract *contract);
    void highlightKeyTerms(const QString &text);

    // UI Components
    QVBoxLayout *m_mainLayout;
    QSplitter *m_mainSplitter;

    // Chat area
    QWidget *m_chatWidget;
    QScrollArea *m_chatScrollArea;
    QVBoxLayout *m_chatLayout;
    QWidget *m_messagesContainer;
    QVBoxLayout *m_messagesLayout;
    QWidget *m_typingIndicator;

    // Input area
    QWidget *m_inputWidget;
    QLineEdit *m_messageInput;
    QPushButton *m_sendButton;
    QPushButton *m_clearButton;
    QPushButton *m_exportButton;

    // Control panel
    QWidget *m_controlPanel;
    QComboBox *m_chatModeCombo;
    QComboBox *m_analysisModeCombo;
    QCheckBox *m_enableSuggestionsCheck;
    QCheckBox *m_enableAnalysisCheck;
    QPushButton *m_settingsButton;

    // Quick actions
    QWidget *m_quickActionsWidget;
    QVBoxLayout *m_quickActionsLayout;
    QPushButton *m_analyzeContractButton;
    QPushButton *m_compareContractsButton;
    QPushButton *m_identifyRisksButton;
    QPushButton *m_generateSummaryButton;
    QPushButton *m_extractTermsButton;

    // Suggestions area
    QWidget *m_suggestionsWidget;
    QVBoxLayout *m_suggestionsLayout;
    QLabel *m_suggestionsLabel;

    // Status area
    QWidget *m_statusWidget;
    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;

    // Data and state
    IContractChatbot *m_chatbot;
    ContractDatabaseManager *m_dbManager;
    const Contract *m_currentContract;
    QList<Contract*> m_contracts;
    QList<Contract*> m_selectedContracts;

    // Chat state
    QString m_currentSessionId;
    QStringList m_conversationHistory;
    QStringList m_currentSuggestions;
    QString m_currentChatMode;
    QString m_currentAnalysisMode;
    bool m_suggestionsEnabled;
    bool m_analysisEnabled;

    // Timers
    QTimer *m_typingTimer;
    QTimer *m_suggestionTimer;

    // Constants
    static constexpr int TYPING_TIMEOUT = 2000; // 2 seconds
    static constexpr int MAX_MESSAGE_LENGTH = 1000;
    static constexpr int MAX_HISTORY_ITEMS = 100;
};

#endif // CONTRACTCHATBOTDIALOG_H
