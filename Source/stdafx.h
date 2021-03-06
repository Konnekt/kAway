/**
  *  kAway2 stdafx
  *
  *  Include file for standard system include files,
  *  or project specific include files that are used frequently,
  *  but are changed infrequently.
  *
  *  Licensed under The GNU Lesser General Public License
  *  Redistributions of files must retain the above copyright notice.
  *
  *  @filesource
  *  @copyright    Copyright (c) 2005-2008 Sijawusz Pur Rahnama
  *  @link         svn://konnekt.info/kaway2/ kAway2 plugin SVN Repo
  *  @version      $Revision$
  *  @modifiedby   $LastChangedBy$
  *  @lastmodified $Date$
  *  @license      http://creativecommons.org/licenses/LGPL/2.1/
  */

// include just once
#pragma once

// allow use of features specific to Windows XP or later.
#ifndef _WIN32_WINNT
  #define _WIN32_WINNT 0x0501
#endif

// exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

// Windows Header Files:
#include <windows.h>

#include <process.h>
#include <commctrl.h>
#include <map>
#include <hash_map>
#include <list>
#include <deque>
#include <stdstring.h>
#include <string>
// #include <sstream>

#pragma comment(lib, "comctl32.lib")

// hapsamy Stamina::Lib
#ifdef _DEBUG
  #pragma comment(lib, "stamina_d.lib")
#else
  #pragma comment(lib, "stamina.lib")
#endif

// nag��wki boosta
#include <boost/signal.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

// nag��wki S.L
/*
#include <stamina/threadrun.h>
#include <stamina/thread.h>
#include <stamina/threadinvoke.h>
#include <stamina/timer.h>
*/

#include <stamina/helpers.h>
#include <stamina/object.h>
#include <stamina/objectimpl.h>
#include <stamina/exception.h>
#include <stamina/string.h>
#include <stamina/time64.h>
#include <stamina/regex.h>
#include <stamina/timer.h>

using namespace Stamina;
using namespace std;

// nag��wki Konnekta
#include <konnekt/plug_export.h>
#include <konnekt/ui.h>
#include <konnekt/plug_func.h>
#include <konnekt/knotify.h>
#include <konnekt/ksound.h>
#include <konnekt/sms.h>
#include <konnekt/tabletka.h>
#include <konnekt/ui_message_controls.h>
#include <konnekt/lib.h>
#include <konnekt/plugsNET.h>

using namespace Konnekt;
using namespace boost;

// plik z definicjami zasob�w
#include "../Project/resource.h"

/**
 * Konnekt::ShowBits helpers
 */
#define ifPRO         if (Konnekt::ShowBits::checkLevel(Konnekt::ShowBits::levelPro))
#define ifADV         if (Konnekt::ShowBits::checkLevel(Konnekt::ShowBits::levelAdvanced))
#define ifNORM        if (Konnekt::ShowBits::checkLevel(Konnekt::ShowBits::levelNormal))
#define ifINT         if (Konnekt::ShowBits::checkLevel(Konnekt::ShowBits::levelIntermediate))

#define ifToolTipADV  if (Konnekt::ShowBits::checkBits(Konnekt::ShowBits::showTooltipsAdvanced))
#define ifToolTipNORM if (Konnekt::ShowBits::checkBits(Konnekt::ShowBits::showTooltipsNormal))
#define ifToolTipBEG  if (Konnekt::ShowBits::checkBits(Konnekt::ShowBits::showTooltipsBeginner))

#define ifInfoADV     if (Konnekt::ShowBits::checkBits(Konnekt::ShowBits::showInfoAdvanced))
#define ifInfoNORM    if (Konnekt::ShowBits::checkBits(Konnekt::ShowBits::showInfoNormal))
#define ifInfoBEG     if (Konnekt::ShowBits::checkBits(Konnekt::ShowBits::showInfoBeginner))

#define ifWizardsADV  if (Konnekt::ShowBits::checkBits(Konnekt::ShowBits::showWizardsAdvanced))
#define ifWizardsNORM if (Konnekt::ShowBits::checkBits(Konnekt::ShowBits::showWizardsNormal))
#define ifWizardsBEG  if (Konnekt::ShowBits::checkBits(Konnekt::ShowBits::showWizardsBeginner))