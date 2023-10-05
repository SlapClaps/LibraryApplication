#pragma once

#include <QDialog>
#include "ui_LoginDialog.h"
#include "Client.h"

class LoginDialog : public QDialog
{
	Q_OBJECT

public:
	LoginDialog(QWidget *parent = nullptr);
	~LoginDialog();

	CustomClient* getClient() {
		return &myClient;
	}

private:
	Ui::LoginDialogClass ui;

	void onLogin();


	CustomClient myClient;
};
