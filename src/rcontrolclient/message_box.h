#pragma once

#include <QMessageBox>
#include <QTimer>
#include <QPushButton>
#include <QApplication>

QMessageBox::StandardButton messageBox(QWidget* parent,
                                       QMessageBox::Icon icon,
                                       QString message,
                                       int closeTimeout = 0);
