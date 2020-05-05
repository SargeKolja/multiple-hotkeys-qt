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
#if QT_VERSION < 0x050000
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

#if QT_VERSION < 0x050000
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
#if QT_VERSION < 0x050000
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
#if QT_VERSION < 0x050000
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


QString MultiHotKey::getAllHotkeys( QAbstractButton *button ) const
{
    QString ShortCutNames;
    QString AltAccelerator;
    int num=0;

    Hotkeys_t::const_iterator it = m_ButtonsAndKeys.begin();
    while( it != m_ButtonsAndKeys.end() )
    {
      QKeySequence KeySequence = it.key();
      QAbstractButton* curr_button = it.value().first;
      QString storedButtonName( it.value().first->text() );

      if( !button || (button == curr_button ) ) // call for one special button or call for all buttons
      {
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

        if( ! AltLetter.isEmpty() && ! ShortCutNames.contains(AltLetter) )
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

    if( num==1 )
    {
      ShortCutNames.prepend( QObject::tr("Hotkey:'") );
      ShortCutNames.append("'");
    }
    else if( num>1 )
    {
      ShortCutNames.prepend( QObject::tr("Hotkeys:'") );
      ShortCutNames.append("'");
    }

    return ShortCutNames;
}



void MultiHotKey::refreshHotkeyTooltip(QAbstractButton* button)
{
    Hotkeys_t::iterator it = m_ButtonsAndKeys.begin();
    while( it != m_ButtonsAndKeys.end() )
    {
      QAbstractButton* curr_button = it.value().first;
      if( !button || button == curr_button ) // call for one special button or call for all buttons
      {
        QString RecentToolTip( m_ButtonsAndTips[ curr_button ] );
        if( RecentToolTip.isEmpty() )
        {  RecentToolTip = button->toolTip();
           m_ButtonsAndTips[ button ] = RecentToolTip;
        }
        curr_button->setToolTip( makeTooltip( RecentToolTip, getAllHotkeys(curr_button) ) );
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
