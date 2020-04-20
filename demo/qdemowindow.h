#ifndef QDEMO_WINDOW_H
#define QDEMO_WINDOW_H

#include <QWidget>
#include <QLabel>
#include <QHash>
#include <QPair>
#include <QPushButton>

#include "../src/multihotkey.h"

class QDemoWindow : public QWidget
{
    Q_OBJECT
public:
    explicit QDemoWindow(QWidget *parent = nullptr);
private slots:
    void on_Button1_clicked(bool checked);
    void on_Button2_clicked(bool checked);
    void on_ButtonChange_clicked(bool checked);

private:
    QLabel* headline;
    QPushButton *m_pButton1; QString m_Button1_Label;
    QPushButton *m_pButton2; QString m_Button2_Label;
    QPushButton *m_pButtonChange;
    MultiHotKey m_Hotkeys;
};

#endif // QDEMO_WINDOW_H
