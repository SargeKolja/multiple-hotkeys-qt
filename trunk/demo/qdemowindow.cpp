#include "qdemowindow.h"

#include <QObject> // at least for tr()
#include <QDebug>
#include <QPushButton>



QDemoWindow::QDemoWindow( QWidget *parent )
  : QWidget(parent)
  , m_Hotkeys( MultiHotKey::HotKeyToolTipOK )
{
    // Set size of the window
    setFixedSize( 20 + 3*80 + 20, 20 + (40 + 30) + 10 );

    headline = new QLabel( " Demo made by sergeantkolja", this );

    // Create and position the button
    m_pButton1 = new QPushButton("H&ello World.", this);  // tickable button
    m_pButton1->setGeometry(10, 40, 80, 30);
    //m_Button1->setCheckable(true); do not make it flipable
    //m_pButton1->setToolTip("This Button is clickable, then returning back to initial state.");
    // <SPC> as a hotkey is always a bad idea, because <SPC> is same as Mouse button on last focused element
    m_Hotkeys.bindKeySequence( QKeySequence( Qt::Key_Backspace ), m_pButton1, tr("This Button is clickable, then returning back to initial state.") );
    m_Hotkeys.bindKeySequence( QKeySequence( Qt::Key_1 )        , m_pButton1 );

    m_pButton2 = new QPushButton("Hi Moo&n!", this);  // flipable button
    m_pButton2->setGeometry(10+80, 40, 80, 30);
    m_pButton2->setCheckable(true);
    //m_pButton2->setToolTip("This Button is 2 times clickable, click one more time to return back to initial state.");
    m_Hotkeys.bindKeySequence( QKeySequence( Qt::Key_Return ), m_pButton2 );
    m_Hotkeys.bindKeySequence( QKeySequence( Qt::Key_Enter ) , m_pButton2 );
    m_Hotkeys.bindKeySequence( QKeySequence( Qt::SHIFT | Qt::Key_Return ) , m_pButton2, tr("This Button is 2 times clickable, click one more time to return back to initial state.") );
    m_Hotkeys.bindKeySequence( QKeySequence( Qt::ALT   | Qt::Key_Return ) , m_pButton2 );
    m_Hotkeys.bindKeySequence( QKeySequence( Qt::CTRL  | Qt::Key_Return ) , m_pButton2 );

    m_pButtonChange = new QPushButton("-> Germ&an", this);
    m_pButtonChange->setGeometry(10+80+80, 40, 80, 30);
    m_pButtonChange->setCheckable(true);
    m_pButtonChange->setToolTip("Toggle between alternative button text.");
    m_Hotkeys.bindKeySequence( QKeySequence( tr("+", "also single signs are allowed for acelerators") ), m_pButtonChange );

    connect( m_pButton1,      SIGNAL(clicked(bool)), this, SLOT(on_Button1_clicked(bool)) );
    connect( m_pButton2,      SIGNAL(clicked(bool)), this, SLOT(on_Button2_clicked(bool)) );
    connect( m_pButtonChange, SIGNAL(clicked(bool)), this, SLOT(on_ButtonChange_clicked(bool)) );  // would be SIGNAL(toggled), but only clicked is bound to accelerators
}


void QDemoWindow::on_Button1_clicked( bool checked )
{
  if( m_Button1_Label.isEmpty() )
    { m_Button1_Label = m_pButton1->text();
    }

  if( checked )
    { m_pButton1->setText("Checked");
    }
  else
    { m_pButton1->setText( m_Button1_Label );
    }

  m_Hotkeys.refreshHotkeyTooltip( m_pButton1 );
  //qDebug() << "Hotkeys of Btn 1:" << m_Hotkeys.getAllHotkeys( m_pButton1 );
}


void QDemoWindow::on_Button2_clicked( bool checked )
{
  if( m_Button2_Label.isEmpty() )
    { m_Button2_Label = m_pButton2->text();
    }

  if( checked )
    { m_pButton2->setText("Checked");
    }
  else
    {   m_pButton2->setText(m_Button2_Label);
    }

  m_Hotkeys.refreshHotkeyTooltip( m_pButton2 );
  //qDebug() << "Hotkeys of Btn 2:" << m_Hotkeys.getAllHotkeys( m_pButton2 );
}


void QDemoWindow::on_ButtonChange_clicked( bool checked )
{
    if( checked )
    {
      m_Button1_Label = "H&allo Welt.";   m_pButton1->setText(m_Button1_Label);
      m_Hotkeys.unbindKeySequence( QKeySequence( Qt::Key_Backspace ), m_pButton1 );
      m_Hotkeys.unbindKeySequence( QKeySequence( Qt::Key_1 ), m_pButton1 );
      m_Hotkeys.bindKeySequence( QKeySequence( Qt::Key_2 ), m_pButton1 );

      m_Button2_Label = "Tach Mon&d!";    m_pButton2->setText(m_Button2_Label);

      m_pButtonChange->setText("-> &Englisch");
    }
    else
    {
      m_Button1_Label = "H&ello World.";  m_pButton1->setText(m_Button1_Label);  // shall intentionally collide with German btnChange
      m_Hotkeys.unbindKeySequences( m_pButton1 );
      m_Hotkeys.bindKeySequence( QKeySequence( Qt::Key_Backspace ), m_pButton1 ); // <SPC> is always a bad idea, because <SPC> is same as Mouse button on last focused element
      m_Hotkeys.bindKeySequence( QKeySequence( tr("1") ), m_pButton1 );

      m_Button2_Label = "Hi Moo&n!";      m_pButton2->setText(m_Button2_Label);
      m_pButtonChange->setText("-> Germ&an");  // shall intentionally collide with German btn1
    }

    m_Hotkeys.refreshHotkeyTooltip( nullptr ); // for all
    //qDebug() << "All Hotkeys active:" << m_Hotkeys.getAllHotkeys( nullptr );
}

