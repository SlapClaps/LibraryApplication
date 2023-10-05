#include "LoginDialog.h"
#include "qpushbutton.h"
#include "qlineedit.h"


LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
    connect(ui.loginButton, &QPushButton::clicked, this, &LoginDialog::onLogin);
    myClient.Connect("127.0.0.1", 60000);
}

LoginDialog::~LoginDialog()
{}

void LoginDialog::onLogin() {
    QString usernameInput = ui.usernameInput->text();
    QString passwordInput = ui.passwordInput->text();
    std::string username = usernameInput.toStdString();
    std::string password = passwordInput.toStdString();
    myClient.handleLogin(username, password);
    std::string tempResponse = myClient.waitForResponse();
    QString response = QString::fromStdString(tempResponse);
    ui.responseLabel->setText(response);
    if (tempResponse != "Denial;") {
        accept();
    }
}