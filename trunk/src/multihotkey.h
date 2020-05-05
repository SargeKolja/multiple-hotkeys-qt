#include <QtGlobal>
#include <functional> // Qt4 needs some help, because it does not know the new Qt5 Signal/Slot extensions
#include <QAbstractButton>
#include <QKeySequence>
#include <QShortcut>
#include <QString>
#include <QHash>
#include <QMap>

#if QT_VERSION < 0x050000
class LambdaWrapper : public QObject
{
  Q_OBJECT
public:
    LambdaWrapper( std::function< void() > lambda, QObject* parent = 0)
      : QObject(parent)
      , m_ptr( lambda )
    {}

    ~LambdaWrapper()
    {}

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
    QString getAllHotkeys( QAbstractButton *button=nullptr ) const; // get human readable string of hotkeys from this or from all buttons
    void refreshHotkeyTooltip( QAbstractButton *button=nullptr ); // refresh human readable string of hotkeys, i.e. after changing ButtonLabel (from this or from all buttons)

private:
    bool bindKeySequence_intern(const QKeySequence &KeySequence, QAbstractButton *button, bool takeToolTip, const QString& Tooltip1st );  // bind this seq to this button
    QString makeTooltip(const QString& Tool1st, const QString& Keylist) const;

    typedef QPair< QAbstractButton*, QShortcut* > Accelerator_t;
    typedef QMap< QKeySequence, Accelerator_t > Hotkeys_t;
#if QT_VERSION < 0x050000
    typedef QMap< QKeySequence, LambdaWrapper* > Lambdas_t;
#endif // QT 4 or 5
    typedef QHash< QAbstractButton*, QString > ToolTips_t;

    Hotkeys_t  m_ButtonsAndKeys;
    ToolTips_t m_ButtonsAndTips;
#if QT_VERSION < 0x050000
    Lambdas_t  m_Lambdas;
#endif // QT 4 or 5
    bool       m_withTooltips;
};
