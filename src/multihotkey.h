#include <QAbstractButton>
#include <QKeySequence>
#include <QShortcut>
#include <QString>


class MultiHotKey
{
public:
    enum Flags { HotKeyToolTipNO=false, HotKeyToolTipOK=true };

    explicit MultiHotKey( bool withTooltip = HotKeyToolTipNO );
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
    typedef QHash< QKeySequence, Accelerator_t > Hotkeys_t;
    typedef QHash< QAbstractButton*, QString > ToolTips_t;

    Hotkeys_t  m_ButtonsAndKeys;
    ToolTips_t m_ButtonsAndTips;
    bool       m_withTooltips;
};
