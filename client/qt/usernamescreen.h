#ifndef USERNAMESCREEN_H
#define USERNAMESCREEN_H

#include <QWidget>

class QLineEdit;

class UsernameScreen : public QWidget {
    Q_OBJECT

public:
    explicit UsernameScreen(QWidget* parent = nullptr);

signals:
    void usernameConfirmed(const QString& username);

private:
    void confirm();

    QLineEdit* input;
};

#endif  // USERNAMESCREEN_H