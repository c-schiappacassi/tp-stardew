#ifndef JOINSCREEN_H
#define JOINSCREEN_H

#include <QWidget>

class QLineEdit;
class QLabel;

class JoinScreen : public QWidget {
    Q_OBJECT

public:
    explicit JoinScreen(QWidget* parent = nullptr);
    void showError(const QString& message);

signals:
    void joinRequested(const QString& inviteCode);
    void backRequested();

private:
    void confirm();

    QLineEdit* input;
    QLabel* errorLabel;
};

#endif  // JOINSCREEN_H