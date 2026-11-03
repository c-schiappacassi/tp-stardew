#ifndef USERNAMESCREEN_H
#define USERNAMESCREEN_H

#include <QWidget>

class QLineEdit;
class QLabel;

class UsernameScreen : public QWidget {
    Q_OBJECT

public:
    explicit UsernameScreen(QWidget* parent = nullptr);
    void showError(const QString& message);

signals:
    void usernameConfirmed(const QString& username);

private:
    void confirm();

    QLineEdit* input;
    QLabel* errorLabel;
};

#endif  // USERNAMESCREEN_H