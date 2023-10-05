#include "QtLibGUI.h"             // Include the header file for your main GUI window.
#include "LoginDialog.h"
#include <QtWidgets/QApplication> // Include the QApplication class, which manages application-wide resources and settings.

int main(int argc, char* argv[])  // Standard C++ main function.
{
    QApplication a(argc, argv);   // Create a QApplication object. This is necessary for any Qt GUI application. It manages GUI control flow and main settings.
    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        QtLibGUI w(loginDialog.getClient());    // Create an instance of your main window class, and pass the client as a pointer.
        w.show();                     // Display the main window.
        return a.exec();              // Enter the main event loop and wait until exit() is called. It's where all events from the window system and other sources are processed and dispatched.
    }
}