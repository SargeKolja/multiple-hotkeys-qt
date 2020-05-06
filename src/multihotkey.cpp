#include <QObject> // at least for tr()
#include <QString>
#include <QAbstractButton>
#include <QKeySequence>
#include <QShortcut>


#include <QDebug>

#include "multihotkey.h"


MultiHotKey::MultiHotKey(bool withTooltip , QObject* parent)
  : QObject(parent)
  , m_withTooltips( withTooltip )
  {
  }


MultiHotKey::~MultiHotKey()
  {
  unbindKeySequences();
  }


// bind this seq to this button, and replace or kill ("") the tooltip
bool MultiHotKey::bindKeySequence(const QKeySequence &KeySequence, QAbstractButton *button, const QString& Tooltip1st )
{ return bindKeySequence_intern( KeySequence, button, true, Tooltip1st );  // bind this seq to this button
}

// bind this seq to this button, but do not touch the tooltip
bool MultiHotKey::bindKeySequence(const QKeySequence &KeySequence, QAbstractButton *button )
{ return bindKeySequence_intern( KeySequence, button, false, QString() );
}



bool MultiHotKey::bindKeySequence_intern(const QKeySequence &KeySequence, QAbstractButton *button, bool takeToolTip, const QString& Tooltip1st )
{
    if( !button )
      return false;

    if( m_ButtonsAndKeys.contains( KeySequence )  )
    {
      Accelerator_t Accel( m_ButtonsAndKeys[ KeySequence ] );
      QString storedButtonName( Accel.first->text() );
      if( button->text() == storedButtonName )
      {
        //qDebug() << "already bound Hotkey" << KeySequence.toString() << "for" << storedButtonName << "/" << button->text();
        return false;
      }
      else
      {
        QShortcut * usedShortcut = Accel.second;
#if !defined(MULTIHOTKEY_LAMBDA)
        usedShortcut->disconnect( SIGNAL(activated()) );
        SlotWrapper* pOldAction = m_Slots[ KeySequence ];
        m_Slots.remove( KeySequence );
        delete pOldAction;
#elif defined(QT_VERSION) && (QT_VERSION < 0x050000)
        usedShortcut->disconnect( SIGNAL(activated()) );
        LambdaWrapper* pOldAction = m_Lambdas[ KeySequence ];
        m_Lambdas.remove( KeySequence );
        delete pOldAction;
#else
        QObject::disconnect( usedShortcut, &QShortcut::activated, nullptr, nullptr );
#endif // has QT4 or 5
        m_ButtonsAndKeys.remove( KeySequence );
        //qDebug() << "removed Hotkey" << KeySequence.toString() << "from" << storedButtonName;
        delete usedShortcut;
      }
    }

    QShortcut *pShortCut = new QShortcut( KeySequence, button );

#if !defined(MULTIHOTKEY_LAMBDA)
    SlotWrapper* pSlotWrapper = new SlotWrapper( button, this );
    QObject::connect( pShortCut, SIGNAL(activated()), pSlotWrapper, SLOT(call()) );
    m_Slots[ KeySequence ] = pSlotWrapper;
#elif defined(QT_VERSION) && (QT_VERSION < 0x050000)
    LambdaWrapper* pLambdaAction = new LambdaWrapper( [button](){ button->animateClick(); }, this );
    QObject::connect( pShortCut, SIGNAL(activated()), pLambdaAction, SLOT(call()) );
    m_Lambdas[ KeySequence ] = pLambdaAction;
#else
    QObject::connect( pShortCut, &QShortcut::activated, [button](){ button->animateClick(); });
#endif // has QT4 or 5

    m_ButtonsAndKeys[ KeySequence ] = Accelerator_t( button, pShortCut );
    if( takeToolTip ) // any string, even "" replaces stored string, but "" makes deletion of tooltip and useToolTip=false kepps it as it is
    { m_ButtonsAndTips[ button ] = Tooltip1st;
      button->setToolTip( makeTooltip( Tooltip1st, getAllHotkeys(button) ) );
    }
    else
    { QString RecentToolTip( m_ButtonsAndTips[ button ] );
      if( RecentToolTip.isEmpty() )
      {  RecentToolTip = button->toolTip();
         m_ButtonsAndTips[ button ] = RecentToolTip;
      }
      button->setToolTip( makeTooltip( RecentToolTip, getAllHotkeys(button) ) );
    }
    //qDebug() << "inserted new Hotkey" << KeySequence.toString() << "for" << button->text();
    return true;
}


bool MultiHotKey::unbindKeySequence( const QKeySequence &KeySequence, QAbstractButton *button )
{
  if( m_ButtonsAndKeys.contains( KeySequence )  )
  {
    Accelerator_t Accel( m_ButtonsAndKeys[ KeySequence ] );
    QString storedButtonName( Accel.first->text() );
    if( !button || (button->text() == storedButtonName ) ) // call for one special button or call for all buttons
    {
      QShortcut * usedShortcut = Accel.second;
#if !defined(MULTIHOTKEY_LAMBDA)
      usedShortcut->disconnect( SIGNAL(activated()) );
      SlotWrapper* pOldAction = m_Slots[ KeySequence ];
      m_Slots.remove( KeySequence );
      delete pOldAction;
#elif defined(QT_VERSION) && (QT_VERSION < 0x050000)
      usedShortcut->disconnect( SIGNAL(activated()) );
      LambdaWrapper* pOldAction = m_Lambdas[ KeySequence ];
      m_Lambdas.remove( KeySequence );
      delete pOldAction;
#else
      QObject::disconnect( usedShortcut, &QShortcut::activated, nullptr, nullptr );
#endif // has QT4 or 5
      m_ButtonsAndKeys.remove( KeySequence );
      //qDebug() << "removed Hotkey" << KeySequence.toString() << "from" << storedButtonName;
      delete usedShortcut;
      // Accel.second=nullptr; not required because Accel is not longer used and QMap element is removed
    }
    return true;
  }
  return false;
}


bool MultiHotKey::unbindKeySequences( QAbstractButton *button )
{
    bool bDone=false;

    Hotkeys_t::iterator it = m_ButtonsAndKeys.begin();
    while( it != m_ButtonsAndKeys.end() )
    {
      QKeySequence KeySequence = it.key();
      QAbstractButton* curr_button = it.value().first;
      QString storedButtonName( it.value().first->text() );
      QShortcut * usedShortcut = it.value().second;

      if( !button || (button == curr_button ) ) // call for one special button or call for all buttons
      {
#if !defined(MULTIHOTKEY_LAMBDA)
        usedShortcut->disconnect( SIGNAL(activated()) );
        SlotWrapper* pOldAction = m_Slots[ KeySequence ];
        m_Slots.remove( KeySequence );
        delete pOldAction;
#elif defined(QT_VERSION) && (QT_VERSION < 0x050000)
        usedShortcut->disconnect( SIGNAL(activated()) );
        LambdaWrapper* pOldAction = m_Lambdas[ KeySequence ];
        m_Lambdas.remove( KeySequence );
        delete pOldAction;
#else
        QObject::disconnect( usedShortcut, &QShortcut::activated, nullptr, nullptr );
#endif // has QT4 or 5
        it = m_ButtonsAndKeys.erase( it );
        //qDebug() << "removed Hotkey" << KeySequence.toString() << "from" << storedButtonName;
        delete usedShortcut;
        bDone=true;
      }
      else
      { it++;
      }
    }
    return bDone;
}


/* ==
 * a) get a human readable String with _all_ hotkeys of the given Button
 * b) get a human readable String with _all_ hotkeys of _all_ Buttons in case of arg1 == NULL (or empty, it's default)
 *    i.e.: "Hotkeys:'Alt+E', '+', 'Alt+A', '2', 'Alt+D', 'Return', 'Enter', 'Shift+Return', 'Ctrl+Return', 'Alt+Return'"
 * no c) if one wants to have all Buttons hotkeys grouped by button, one need to call this from iteration:
 *    QStringList MultiHotKey::getAllHotkeysByButton(void)
 * == */
QString MultiHotKey::getAllHotkeys( QAbstractButton *button, bool TooltipFriendlyFormat ) const
{
    QString ShortCutNames;
    if( !button )
    {
      return ShortCutNames;
    }

    QString AltAccelerator, storedButtonName;
    int num=0;

    // we don not maintain a list 'per button', but a list 'per hotkey' which contains 1...n hotkeys pointing to a button - thats why we need to loop all
    Hotkeys_t::const_iterator it = m_ButtonsAndKeys.begin();
    while( it != m_ButtonsAndKeys.end() )
    {
      QAbstractButton* curr_button = it.value().first;

      if( button == curr_button ) // found the requested button
      {
        QKeySequence KeySequence = it.key();
        storedButtonName = it.value().first->text();

        QString AltLetter;
        int from=0, ampersand, twice;
        //qDebug() << "found Hotkey" << KeySequence.toString() << "from" << storedButtonName;
        num++;
        do
        {
          ampersand = storedButtonName.indexOf( '&', from );
          twice = storedButtonName.indexOf( "&&", from );
          if( ampersand>-1 && ampersand==twice ) from++;
          if( ampersand>-1 && ampersand!=twice && ampersand<storedButtonName.length()-1 )
          {
            QString Letter( storedButtonName[ampersand+1] );
            AltLetter.append("Alt+").append( Letter.toUpper() );
            break;
          }
        } while( ampersand>-1 && ++from < storedButtonName.length() );

        if( ! AltLetter.isEmpty() && ! ShortCutNames.contains(AltLetter) ) // ensure, the Alt+... Letter is only inserted once and only at 1st position
        {
          if( ! ShortCutNames.isEmpty() )
          { ShortCutNames.append("', '");
          }
          ShortCutNames.append( AltLetter );
        }

        if( ! ShortCutNames.isEmpty() )
        { ShortCutNames.append("', '");
        }
        ShortCutNames.append( KeySequence.toString() );
      }
      it++;
    }

    if( TooltipFriendlyFormat )
    {
        QString Line( ( num==1 ) ? \
                    (QObject::tr("Hotkey:'%1'").arg(ShortCutNames)) : \
                    (QObject::tr("Hotkeys:'%1'").arg(ShortCutNames)) );
        return Line;
    }
    else
    {
        QString Line( ( num==1 ) ? \
                    (QObject::tr("Hotkey('%1'):'%2'").arg(storedButtonName).arg(ShortCutNames)) : \
                    (QObject::tr("Hotkeys('%1'):'%2'").arg(storedButtonName).arg(ShortCutNames)) );
        return Line;
    }
}


QStringList MultiHotKey::getAllHotkeysByButton( bool TooltipFriendlyFormat ) const
{
  QStringList ListOfHotkeys;

  Hotkeys_t::const_iterator it = m_ButtonsAndKeys.begin();
  QStringList seen;
  while( it != m_ButtonsAndKeys.end() )
  {
    QAbstractButton* curr_button = it.value().first;
    QString curr_imprint( it.value().first->text() );
    if( ! seen.contains( curr_imprint ) ) // only evaluate the getAllHotkeys 1 times for each button, while this loop is for each hotkey
    {
      QString HotKeys = getAllHotkeys( curr_button, TooltipFriendlyFormat ) + "\n";
      ListOfHotkeys.push_back( HotKeys );
      seen.push_back( curr_imprint );
    }
    ++it;
  }
  return ListOfHotkeys;
}


void MultiHotKey::refreshHotkeyTooltip(QAbstractButton* button, const QString& setTooltip)
{
    Hotkeys_t::iterator it = m_ButtonsAndKeys.begin();
    while( it != m_ButtonsAndKeys.end() )
    {
      QAbstractButton* curr_button = it.value().first;
      if( !button || button == curr_button ) // call for one special button or call for all buttons
      {
        QString RecentToolTip( setTooltip.isEmpty() ? m_ButtonsAndTips[ curr_button ] : setTooltip );
        if( RecentToolTip.isEmpty() )
        {  RecentToolTip = curr_button->toolTip();
        }
        m_ButtonsAndTips[ curr_button ] = RecentToolTip;
        QString newTooltip( makeTooltip( RecentToolTip, getAllHotkeys(curr_button) ) );
        curr_button->setToolTip( newTooltip );
      }
      it++;
    }
    return;
}


QString MultiHotKey::makeTooltip( const QString& Tool1st, const QString& Keylist ) const
{
  QString Result, HotKeyNames, Tooltip1st(Tool1st);
  if( m_withTooltips )
  { HotKeyNames=Keylist;
  }

  if( ! Tooltip1st.isEmpty() && ! Keylist.isEmpty() && ! Tooltip1st.endsWith('\n') )
  { Tooltip1st.append('\n');
  }
  if( ! Tooltip1st.isEmpty() || ! Keylist.isEmpty() )
  { Result = Tooltip1st + Keylist;
  }
  return Result;
}
