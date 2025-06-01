#ifndef CHATBOTDIALOG_H
#define CHATBOTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QTimer>
#include <QKeyEvent>
#include "chatbotmanager.h"

class ChatbotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatbotDialog(ChatbotManager* chatbotManager, QWidget *parent = nullptr);
    ~ChatbotDialog();

    void setSelectedContractId(const QString &contractId);

private slots:
    void sendMessage();
    void onResponseReady(const QString &response);
    void onErrorOccurred(const QString &error);
    void showTypingIndicator();
    void hideTypingIndicator();

private:
    void setupUI();
    void addMessage(const QString &message, bool isUser);
    void scrollToBottom();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    
private:// UI Components
    QVBoxLayout *m_mainLayout;
    QScrollArea *m_scrollArea;
    QWidget *m_chatContainer;
    QVBoxLayout *m_chatLayout;
    QHBoxLayout *m_inputLayout;
    QLineEdit *m_inputField;
    QPushButton *m_sendButton;
    QLabel *m_typingIndicator;
    
    // Data
    ChatbotManager *m_chatbotManager;
    QString m_selectedContractId;
    QTimer *m_typingTimer;
    
    // Styling constants
    static const QString DIALOG_STYLE;
    static const QString MESSAGE_USER_STYLE;
    static const QString MESSAGE_BOT_STYLE;
    static const QString INPUT_STYLE;
    static const QString BUTTON_STYLE;
};

#endif // CHATBOTDIALOG_H
