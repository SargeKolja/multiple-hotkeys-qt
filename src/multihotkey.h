#ifndef MULTIHOTKEY_H
#define MULTIHOTKEY_H


#include <QtGlobal>

#if defined( MULTIHOTKEY_LAMBDA )
#  include <functional> // Qt4 needs some help, because it does not know the new Qt5 Signal/Slot extensions
#endif

#include <QAbstractButton>
#include <QKeySequence>
#include <QShortcut>
#include <QString>
#include <QHash>
#include <QMap>

#if !defined( MULTIHOTKEY_LAMBDA )
class SlotWrapper : public QObject
{
  Q_OBJECT
public:
    SlotWrapper( QAbstractButton *button, QObject* parent = 0)
      : QObject(parent)
      , m_pButton(button)
    {}

    ~SlotWrapper()
    {
      disconnect( this, SLOT(call()) );
    }

public slots:
    void call() { if( m_pButton ) m_pButton->animateClick(); }

private:
  QAbstractButton* m_pButton;  // must be compatible to: void QAbstractButton::animateClick( int msec=100 ), we choose: void foo(void)
};

#elif defined(QT_VERSION) && (QT_VERSION < 0x050000)

class LambdaWrapper : public QObject
{
  Q_OBJECT
public:
    LambdaWrapper( std::function< void() > lambda, QObject* parent = 0)
      : QObject(parent)
      , m_ptr( lambda )
    {}

    ~LambdaWrapper()
    {
      disconnect( this, SLOT(call()) );
    }

public slots:
    void call() { if( m_ptr ) m_ptr(); }

private:
  std::function< void() > m_ptr;  // must be compatible to: void QAbstractButton::animateClick( int msec=100 ), we choose: void foo(void)
};
#endif // QT 4 or 5


class MultiHotKey : public QObject
{
  Q_OBJECT
public:
    enum Flags { HotKeyToolTipNO=false, HotKeyToolTipOK=true };

    explicit MultiHotKey( bool withTooltip = HotKeyToolTipNO, QObject* parent = 0 );
    ~MultiHotKey();

    bool bindKeySequence(const QKeySequence &KeySequence, QAbstractButton *button, const QString& Tooltip1st );  // bind this seq to this button
    bool bindKeySequence(const QKeySequence &KeySequence, QAbstractButton *button );                             // bind this seq to this button
    bool unbindKeySequence( const QKeySequence &KeySequence, QAbstractButton *button=nullptr );  // remove this seq from this or from any button
    bool unbindKeySequences( QAbstractButton *button=nullptr ); // remove all sequences from this or from all buttons
    QString getAllHotkeys(QAbstractButton *button, bool TooltipFriendlyFormat=true ) const; // get human readable string of hotkeys all from only this button, false is for verbose format (debug or log)
    QStringList getAllHotkeysByButton( bool TooltipFriendlyFormat=false ) const; // get human readable string list of all hotkeys from all registered buttons
    void refreshHotkeyTooltip( QAbstractButton *button=nullptr, const QString& setTooltip = QString() ); // refresh human readable string of hotkeys, i.e. after changing ButtonLabel (from this or from all buttons)

private:
    bool bindKeySequence_intern(const QKeySequence &KeySequence, QAbstractButton *button, bool takeToolTip, const QString& Tooltip1st );  // bind this seq to this button
    QString makeTooltip(const QString& Tool1st, const QString& Keylist) const;

    typedef QPair< QAbstractButton*, QShortcut* > Accelerator_t;
    // to use QHash with QKeySequence also in Qt4, you need to declare: uint qHash(const QKeySequence &key, uint seed = 0) noexcept;
    typedef QMap< QKeySequence, Accelerator_t > Hotkeys_t;
#if !defined(MULTIHOTKEY_LAMBDA)
    typedef QMap< QKeySequence, SlotWrapper* > SlotsWrap_t;
#elif defined(QT_VERSION) && (QT_VERSION < 0x050000)
    typedef QMap< QKeySequence, LambdaWrapper* > Lambdas_t;
#endif // QT 4 or 5
    typedef QHash< QAbstractButton*, QString > ToolTips_t;

    Hotkeys_t  m_ButtonsAndKeys;
    ToolTips_t m_ButtonsAndTips;
#if !defined(MULTIHOTKEY_LAMBDA)
    SlotsWrap_t m_Slots;
#elif defined(QT_VERSION) && (QT_VERSION < 0x050000)
    Lambdas_t  m_Lambdas;
#endif // QT 4 or 5
    bool       m_withTooltips;
};

#endif // MULTIHOTKEY_H
