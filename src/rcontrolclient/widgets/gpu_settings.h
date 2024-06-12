#pragma once

#include "commands/commands.h"
#include "pproto/func_invoker.h"
#include "pproto/transport/tcp.h"

#include <QDialog>
#include <QCloseEvent>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QIntValidator>
#include <QKeyEvent>

using namespace pproto;
using namespace pproto::transport;

namespace Ui {
class GPUSettings;
}

class GPUSettings : public QDialog
{
    Q_OBJECT

signals:
    void signalUpdateSetts(const data::gpuData&);

public:
    explicit GPUSettings(QWidget *parent = nullptr);
    ~GPUSettings();

    bool init(const data::gpuData& newData, tcp::Socket::Ptr socket);

private:
    Ui::GPUSettings *ui;

    data::gpuData _gpuData;
    tcp::Socket::Ptr _socket;
    QString _hoverStyle;
    QString _btnStyle;
    QString _lineStyle;

private:
    void setID();
    void setModel();
    void setMemSize();
    void setFAN();
    void setPL();
    void setCoreClock();
    void setMemoryClock();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject *, QEvent *) override;

private slots:
    void on_btnAccept_clicked(bool checked);
    void on_btnCancel_clicked(bool checked);
    void on_lineFan_textEdited(const QString &arg1);
    void on_linePL_textEdited(const QString &arg1);
    void on_lineMemory_textEdited(const QString &arg1);
    void on_lineCore_textEdited(const QString &arg1);
};
