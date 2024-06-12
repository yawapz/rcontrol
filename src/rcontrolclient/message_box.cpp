#include "message_box.h"

class TimedMessageBox : public QMessageBox
{
public:
    TimedMessageBox(QWidget* parent, int closeTimeout = 0);

private:
    void showEvent(QShowEvent*) override final;

    int _closeTimeout = {0};
    QTimer _closeTimer;
    QString _buttonText;
};

TimedMessageBox::TimedMessageBox(QWidget* parent, int closeTimeout)
    : QMessageBox(parent)
    , _closeTimeout(closeTimeout)
{
    setWindowModality(Qt::WindowModal);
    setWindowFlag(Qt::WindowTitleHint);
    setWindowFlag(Qt::FramelessWindowHint);
    setStyleSheet(
    " QMessageBox           "
    " {                     "
    " border-style: solid;  "
    " border-width: 1px;    "
    " border-color: black;  "
    " }                     "
    );

    QString hoverStyle = " QPushButton:hover                          "
                         " {                                          "
                         " padding: 3px;                              "
                         " border-style: outset;                      "
                         " border-width: 1px;                         "
                         " border-radius: 3px;                        "
                         " border-color: rgba(35, 146, 220, 0.5);     "
                         " font-size: 16px;                           "
                         " background-color: rgba(35, 146, 220, 0.5); "
                         " }                                          ";

    QPushButton* btnOk = addButton(QMessageBox::Ok);
    btnOk->setStyleSheet(hoverStyle);
    if (closeTimeout > 0)
    {
        if (btnOk)
            _buttonText = btnOk->text();

        _closeTimer.setInterval(1000);
        connect(&_closeTimer, &QTimer::timeout, this, [this]()
        {
            if (--_closeTimeout >= 0)
            {
                if (QAbstractButton* btn = this->button(QMessageBox::Ok))
                {
                    QString newBtnTxt = QString("%1 (%2)").arg(_buttonText)
                                                          .arg(_closeTimeout);
                    btn->setText(newBtnTxt);
                }
            }
            else
            {
                _closeTimer.stop();
                this->accept();
            }
        });
    }
}

void TimedMessageBox::showEvent(QShowEvent* e)
{
    QMessageBox::showEvent(e);

    if (_closeTimeout > 0)
        _closeTimer.start();
}

QMessageBox::StandardButton messageBox(QWidget* parent,
                                       QMessageBox::Icon icon,
                                       QString message,
                                       int closeTimeout)
{
    message.replace("  ", "<br>").replace(" ", "&nbsp;");
    TimedMessageBox msgBox {parent, closeTimeout};

    msgBox.setIcon(icon);
    msgBox.setWindowTitle(qApp->applicationName());
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(message);

    int res = msgBox.exec();
    return static_cast<QMessageBox::StandardButton>(res);
}
