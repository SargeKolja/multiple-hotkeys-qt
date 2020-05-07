#include <QtGlobal> // Q_ASSERT
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


QAbstractButton* MultiHotKey::registerButton( QAbstractButton* button )
{
  if( button )
  {
    m_AllButtons.insert( button );
    return registerToolTip( button, getCustomToolTip( button ) );
  }
  return button;
}


QAbstractButton* MultiHotKey::registerToolTip( QAbstractButton* button, const QString& Tooltip1st )
{
  if( button )
  {
    m_AllButtons.insert( button );
    m_ButtonsAndTips[ button ] = Tooltip1st;
    button->setToolTip( makeTooltip( Tooltip1st, getAllHotkeys( button ) ) );
  }
  return button;
}


// bind this seq to this button, and replace or kill ("") the tooltip
bool MultiHotKey::bindKeySequence(const QKeySequence &KeySequence, QAbstractButton *button, const QString& Tooltip1st )
{ return bindKeySequence_intern( KeySequence, button, true, Tooltip1st );  // bind this seq to this button
}

// bind this seq to this button, but do not touch the tooltip
bool MultiHotKey::bindKeySequence(const QKeySequence &KeySequence, QAbstractButton *button )
{ return bindKeySequence_intern( KeySequence, button, false, QString() );
}



bool MultiHotKey::bindKeySequence_intern(const QKeySequence &KeySequence, QAbstractButton *button, bool takeTooltip1st, const QString& Tooltip1st )
{
    if( !button )
      return false;

    if( ! KeySequence.isEmpty() && m_ButtonsAndKeys.contains( KeySequence )  )
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
    } // if( ! KeySequence.isEmpty() && m_ButtonsAndKeys.contains( KeySequence )  )

    if( ! KeySequence.isEmpty() )
    {
      QShortcut *pShortCut = 0;
      try
      {
        pShortCut = new QShortcut( KeySequence, button );
        m_AllButtons.insert( button );
      } catch( ... )
      {
        qDebug() << "invalid Key sequence" << KeySequence.toString() << "for" << button->text();
        return false;
      }

#if   !defined(MULTIHOTKEY_LAMBDA)
      SlotWrapper* pSlotWrapper = new SlotWrapper( button, this );
      QObject::connect( pShortCut, SIGNAL(activated()), pSlotWrapper, SLOT(call()) );
      m_Slots[ KeySequence ] = pSlotWrapper;
#     elif defined(QT_VERSION) && (QT_VERSION < 0x050000)
      LambdaWrapper* pLambdaAction = new LambdaWrapper( [button](){ button->animateClick(); }, this );
      QObject::connect( pShortCut, SIGNAL(activated()), pLambdaAction, SLOT(call()) );
      m_Lambdas[ KeySequence ] = pLambdaAction;
#     else
      QObject::connect( pShortCut, &QShortcut::activated, [button](){ button->animateClick(); });
#     endif // has QT4 or 5

      m_ButtonsAndKeys[ KeySequence ] = Accelerator_t( button, pShortCut );
    } // if( KeySequence.isEmpty() )

    if( takeTooltip1st ) // any string, even "" replaces stored string, but "" makes deletion of tooltip and useToolTip=false kepps it as it is
    { m_ButtonsAndTips[ button ] = Tooltip1st;
      button->setToolTip( makeTooltip( Tooltip1st, getAllHotkeys(button) ) );
    }
    else
    { QString RecentToolTip( m_ButtonsAndTips[ button ] );
      if( RecentToolTip.isEmpty() )
      {  RecentToolTip = getCustomToolTip( button );
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

QString MultiHotKey::getAccelerator( const QString& ButtonName ) const
{
  QString AltAccelerator;

  QString AltLetter;
  int from=0, ampersand, twice;

  //qDebug() << "found Hotkey" << KeySequence.toString() << "from" << storedButtonName;
  do
  {
    ampersand = ButtonName.indexOf( '&', from );
    twice = ButtonName.indexOf( "&&", from );
    if( ampersand>-1 && ampersand==twice ) from++;
    if( ampersand>-1 && ampersand!=twice && ampersand < ButtonName.length()-1 )
    {
      QString Letter( ButtonName[ ampersand+1 ] );
      AltLetter.append("Alt+").append( Letter.toUpper() );
      break;
    }
  } while( ampersand>-1 && ++from < ButtonName.length() );
  return AltLetter;
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
        storedButtonName = curr_button->text();

        QString AltLetter = getAccelerator( storedButtonName );
        if( ! AltLetter.isEmpty() )
        { num++;
        }

        if( ! AltLetter.isEmpty() && ! ShortCutNames.contains(AltLetter) ) // ensure, the Alt+... Letter is only inserted once and only at 1st position
        {
          if( ! ShortCutNames.isEmpty() )
          { ShortCutNames.append("', '");
          }
          ShortCutNames.append( AltLetter );
        }

        if( ! KeySequence.isEmpty() )
        {
          if( ! ShortCutNames.isEmpty() )
          { ShortCutNames.append("', '");
          }
          num++;
          ShortCutNames.append( KeySequence.toString() );
        }
      }
      it++;
    }

    if( num==0 ) // nothing found in m_ButtonsAndKeys, but the Button can just have a "La&bel", but no Hotkeys, so ask the button itself:
    {
      storedButtonName = button->text();
      QString AltLetter = getAccelerator( storedButtonName );
      if( ! AltLetter.isEmpty() )
      {
        ShortCutNames.append( AltLetter );
        num++;
      }
    }

    if( TooltipFriendlyFormat )
    {
        QString Line( ( num==0 ) ? \
                    (tr("No Hotkey")) : ( num==1 ) ? \
                    (tr("Hotkey:'%1'").arg(ShortCutNames)) : \
                    (tr("Hotkeys:'%1'").arg(ShortCutNames)) );
        return Line;
    }
    else
    {
      QString Line( ( num==0 ) ? \
                    (tr("Hotkey('%1'): none").arg(storedButtonName)) : ( num==1 ) ? \
                    (tr("Hotkey('%1'):'%2'").arg( storedButtonName).arg(ShortCutNames)) : \
                    (tr("Hotkeys('%1'):'%2'").arg(storedButtonName).arg(ShortCutNames)) );
        return Line;
    }
}


QStringList MultiHotKey::getAllHotkeysByButton( bool TooltipFriendlyFormat ) const
{
  QStringList ListOfHotkeys;
  QStringList seen;

  /* scope_0 */
  {
    Q_FOREACH( QAbstractButton* curr_button, m_AllButtons )
    {
      QString curr_imprint( curr_button->text() );
      if( ! seen.contains( curr_imprint ) ) // only evaluate the getAllHotkeys 1 times for each button, while this loop is for each hotkey
      {
        QString HotKeys = getAllHotkeys( curr_button, TooltipFriendlyFormat );
        if( ! TooltipFriendlyFormat ) HotKeys.append( "\n" );
        ListOfHotkeys.push_back( HotKeys );
        seen.push_back( curr_imprint );
      }
    }
  }

  /* scope_1 should be never hit anything now, right? */
  {
    Hotkeys_t::const_iterator it = m_ButtonsAndKeys.begin();
    while( it != m_ButtonsAndKeys.end() )
    {
      QAbstractButton* curr_button = it.value().first;
      // can get outdated: QString curr_imprint( it.value().first->text() );
      QString curr_imprint( curr_button->text() );
      if( ! curr_imprint.isEmpty() && ! seen.contains( curr_imprint ) ) // only evaluate the getAllHotkeys 1 times for each button, while this loop is for each hotkey
      {
        Q_ASSERT_X( false, curr_imprint.toLatin1().constData(), "button missed in m_AllButtons set, but found in m_ButtonsAndKeys." );
        QString HotKeys = getAllHotkeys( curr_button, TooltipFriendlyFormat );
        if( ! TooltipFriendlyFormat ) HotKeys.append( "\n" );
        ListOfHotkeys.push_back( HotKeys );
        seen.push_back( curr_imprint );
      }
      ++it;
    }
  }

  // in case we do not assigned Hotkeys, we still can have those "Alt+Ampersand-Letter" Accellerators, managed within m_ButtonsAndTips (which is far from perfect, because if we do not assign a hotkey and not a Tip, we are blind for the accellerator - to be improved
  /* scope_2 should be never hit anything now, right? */
  {
    ToolTips_t::const_iterator it2 = m_ButtonsAndTips.begin();
    while( it2 != m_ButtonsAndTips.end() )
    {
      QAbstractButton* curr_button = it2.key();
      //QString curr_imprint( it2.value() ); // this is the tooltip text, not the Imprint
      QString curr_imprint( curr_button->text() );
      if( ! curr_imprint.isEmpty() && ! seen.contains( curr_imprint ) ) // only evaluate the getAllHotkeys 1 times for each button, while this loop is for each hotkey
      {
        Q_ASSERT_X( false, curr_imprint.toLatin1().constData(), "button missed in m_AllButtons set, but found in m_ButtonsAndTips." );
        QString HotKeys = getAllHotkeys( curr_button, TooltipFriendlyFormat );
        if( ! TooltipFriendlyFormat ) HotKeys.append( "\n" );
        ListOfHotkeys.push_back( HotKeys );
        seen.push_back( curr_imprint );
      }
      ++it2;
    }
  }
  return ListOfHotkeys;
}


void MultiHotKey::setToolTip(QAbstractButton* button, const QString& newToolTip)
{
  refreshHotkeyTooltip_internal( button, newToolTip, false );
}

void MultiHotKey::refreshHotkeyTooltip( QAbstractButton* button )
{
  refreshHotkeyTooltip_internal( button, "", true );
}

void MultiHotKey::refreshHotkeyTooltip_internal(QAbstractButton* button, const QString& newToolTip, bool bInheritButtonTooltip )
{
  bool bFoundInButtonsAndTips = false;
  ToolTips_t::iterator it = m_ButtonsAndTips.begin();
  BtnLst_t SeenByTooltip;

  while( it != m_ButtonsAndTips.end() )
  {
    QAbstractButton* curr_button = it.key();
    SeenByTooltip.insert( curr_button );

    if( !button || button == curr_button ) // call for one special button or call for all buttons
    {
      /* if arg2 given, take it.
       * else if m_ButtonsAndTips has stored one, take that
       * else take last Qt API assigned tooltip
       */
      QString RecentToolTip( (! newToolTip.isEmpty()) ? newToolTip : it.value() );
      if( bInheritButtonTooltip && RecentToolTip.isEmpty() )
      { RecentToolTip = getCustomToolTip( button );
      }

      it->operator=( RecentToolTip );
      //m_ButtonsAndTips[ curr_button ] = RecentToolTip;
      QString newTooltip( makeTooltip( RecentToolTip, getAllHotkeys(curr_button) ) );
      curr_button->setToolTip( newTooltip );
      bFoundInButtonsAndTips = true;
    }
    it++;
  }

  if( !button ) // refreshed all, but there might be some only registered, not tooltipped, not hotkeyed buttons with an not yet seen accellerator
  {
    BtnLst_t RemainingOfAll( m_AllButtons );
    RemainingOfAll.subtract( SeenByTooltip );
    Q_FOREACH( QAbstractButton* rem, RemainingOfAll )
    {
      QString newTooltip( makeTooltip( "", getAllHotkeys(rem) ) );
      rem->setToolTip( newTooltip );
    }
  }

  // currently empty container - typically this is the 1st time we assign a tooltip, before assigning a hotkey!
  if( button && ! bFoundInButtonsAndTips )
  {
      m_ButtonsAndTips[ button ] = newToolTip;
      button->setToolTip( newToolTip );
      return;
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


QString MultiHotKey::getCustomToolTip( const QAbstractButton* button ) const
{
  QString Helper;
  if( button )
  {
    Helper = button->toolTip();
    Helper = Helper.split( tr("No Hotkey") ).first();
    Helper = Helper.split( tr("Hotkey:'" ) ).first();
    Helper = Helper.split( tr("Hotkeys:'") ).first();
  }
  return Helper;
}
