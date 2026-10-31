#ifndef JOINSCREEN_H
#define JOINSCREEN_H

#include <QWidget>

class QLineEdit;

class JoinScreen : public QWidget {
    Q_OBJECT

public:
    explicit JoinScreen(QWidget* parent = nullptr);

signals:
    void joinRequested(const QString& inviteCode);
    void backRequested();

private:
    void confirm();

    QLineEdit* input;
};

#endif  // JOINSCREEN_H