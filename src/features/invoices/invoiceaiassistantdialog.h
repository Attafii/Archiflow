#ifndef INVOICEAIASSISTANTDIALOG_H
#define INVOICEAIASSISTANTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QTextBrowser>
#include <QSplitter>
#include <QProgressBar>
#include <QMenu>
#include <QAction>
#include <QListWidget>
#include <QFileDialog>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include "../materials/groqclient.h"

class DatabaseService;
class Invoice;
class Client;
class InvoiceDatabaseManager;

class InvoiceChatBubble : public QFrame
{
    Q_OBJECT

public:
    enum Type { User, Assistant, System };
    
    explicit InvoiceChatBubble(Type type, const QString &message, QWidget *parent = nullptr);
    void setMessage(const QString &message);
    void setTimestamp(const QDateTime &timestamp);
    
    void animateIn();
    void startTypingAnimation();
    void stopTypingAnimation();
    
private:
    void setupUI();
    void setupAnimations();
      Type m_type;
    QString m_message;
    QDateTime m_timestamp;
    QLabel *m_messageLabel;
    QLabel *m_timestampLabel;
    QLabel *m_avatarLabel;
    QPropertyAnimation *m_fadeInAnimation;
    QPropertyAnimation *m_slideInAnimation;
    QTimer *m_typingTimer;
    QGraphicsOpacityEffect *m_opacityEffect;
    int m_typingDots;
};

class InvoiceTypingIndicator : public QWidget
{
    Q_OBJECT

public:
    explicit InvoiceTypingIndicator(QWidget *parent = nullptr);
    
    void start();
    void stop();
    
public slots:
    void show();
    void hide();
    
private slots:
    void updateAnimation();
    void updateDots();
    
private:
    void setupUI();
    
    QLabel *m_label;
    QTimer *m_timer;
    bool m_isActive;
    int m_dotCount;
    
    QHBoxLayout *m_layout;
    QLabel *m_dot1;
    QLabel *m_dot2;
    QLabel *m_dot3;
    QTimer *m_animationTimer;
    int m_currentDot;
    QPropertyAnimation *m_animation1;
    QPropertyAnimation *m_animation2;
    QPropertyAnimation *m_animation3;
};

class InvoiceAIAssistantDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InvoiceAIAssistantDialog(QWidget *parent = nullptr);
    ~InvoiceAIAssistantDialog();
    
    void setGroqClient(GroqClient *client);
    void setDatabaseService(DatabaseService *dbService);
    void setInvoiceDatabaseManager(InvoiceDatabaseManager *dbManager);
    void setInvoiceContext(const QJsonObject &context);
    void setCurrentInvoice(const Invoice *invoice);
    void setCurrentClient(const Client *client);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void sendMessage();
    void onMessageReceived(const QString &message, const QString &messageId);
    void onErrorOccurred(const QString &error, int code);
    void onConnectionStatusChanged(bool connected);
    void onRequestStarted();
    void onRequestFinished();
    void onTypingStarted();
    void onTypingFinished();
    
    void clearChat();
    void exportChat();
    void showSettings();
    void onQuickActionClicked();
    void attachDocument();
    void onDocumentSelected(const QString &filePath);
    void analyzeDocument(const QString &filePath);
    void processDatabaseCommand(const QString &command);
    void handleDatabaseQuery(const QString &query);
    
protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    
private:
    void setupUI();
    void setupAnimations();
    void setupConnections();
    void setupQuickActions();
    void addMessage(InvoiceChatBubble::Type type, const QString &message);
    void scrollToBottom();
    void updateConnectionIndicator();
    void animateDialogEntry();
    void applyArchiFlowTheme();
    QString formatMessage(const QString &message);
    void saveSettings();
    void loadSettings();
    QString generateAnalysisPrompt(const QString &fileName, const QString &content);
    void showDocumentPreview(const QString &fileName, const QString &content);
    QString processUserMessage(const QString &message);
    QString handleDatabaseOperation(const QString &operation, const QJsonObject &params);
    void addInvoiceQuickActions();
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    QFrame *m_headerFrame;
    QLabel *m_titleLabel;
    QLabel *m_connectionIndicator;
    QPushButton *m_settingsButton;
    QPushButton *m_closeButton;
    
    QScrollArea *m_chatScrollArea;
    QWidget *m_chatWidget;
    QVBoxLayout *m_chatLayout;
    
    QFrame *m_inputFrame;
    QLineEdit *m_messageInput;
    QPushButton *m_sendButton;
    QPushButton *m_attachButton;
    QPushButton *m_voiceButton;
    
    QFrame *m_sidebarFrame;
    QVBoxLayout *m_sidebarLayout;
    QLabel *m_quickActionsLabel;
    QListWidget *m_quickActionsList;
    
    InvoiceTypingIndicator *m_typingIndicator;
    QProgressBar *m_progressBar;
    
    // Data & Logic
    GroqClient *m_groqClient;
    DatabaseService *m_databaseService;
    InvoiceDatabaseManager *m_invoiceDbManager;
    QList<ChatMessage> m_chatHistory;
    QJsonObject m_invoiceContext;
    const Invoice *m_currentInvoice;
    const Client *m_currentClient;
    
    // Animations
    QPropertyAnimation *m_fadeInAnimation;
    QPropertyAnimation *m_scaleAnimation;
    QPropertyAnimation *m_slideInAnimation;
    QGraphicsOpacityEffect *m_opacityEffect;
    
    // Settings
    bool m_autoScroll;
    bool m_showTimestamps;
    QString m_fontFamily;
    int m_fontSize;
    
    // Quick Actions
    QStringList m_quickActions;
};

#endif // INVOICEAIASSISTANTDIALOG_H
