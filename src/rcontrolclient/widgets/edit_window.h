#pragma once

#include "commands/commands.h"
#include "pproto/func_invoker.h"
#include "pproto/transport/tcp.h"
#include "message_box.h"
#include "qclipboard.h"

#include <QDialog>
#include <QPainter>
#include <QCloseEvent>
#include <QPainterPath>
#include <QValidator>
#include <QLocale>
#include <QGraphicsDropShadowEffect>

using namespace pproto;
using namespace pproto::transport;

namespace Ui {
class EditWindow;
}

class EditWindow : public QDialog
{
    Q_OBJECT

public:
    explicit EditWindow(QWidget *parent = nullptr);
    ~EditWindow();

    bool init(tcp::Socket::Ptr socket, const data::workerData&, const QStringList&);

private slots:
    void on_btnUpdate_clicked(bool checked);

private:
    Ui::EditWindow *ui;

    tcp::Socket::Ptr _socket;
    QUuidEx _id;
    QStringList _workerNames;

    QString _hoverStyle;
    QString _btnStyle;
    QString _lineStyle;



protected:
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject *, QEvent *) override;
};
