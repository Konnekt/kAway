/**
  *  SMS Forwarder class
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

#include "stdafx.h"

#include "SMSForwarder.h"
#include "../Controller.h"

namespace kAway2 {
  SMSForwarder::SMSForwarder() : Forwarder("sms", "SMS", 501, true, true) {
    IMessageDispatcher& dispatcher = Context::getInstance()->getIMessageDispatcher();
    ActionDispatcher& action_dispatcher = Context::getInstance()->getActionDispatcher();

    dispatcher.connect(IM_SETCOLS, bind(resolve_cast0(&SMSForwarder::onISetCols), this));
    dispatcher.connect(IM_UI_PREPARE, bind(resolve_cast0(&SMSForwarder::onIPrepare), this));
    dispatcher.connect(IM_MSG_RCV, bind(&SMSForwarder::onNewMsg, this, _1)); // baaad
    dispatcher.connect(im::away, bind(resolve_cast0(&SMSForwarder::onEnable), this));
    dispatcher.connect(im::back, bind(resolve_cast0(&SMSForwarder::onDisable), this));

    action_dispatcher.connect(ui::sms::number, bind(&SMSForwarder::refreshCombo, this, _1));
  }

  void SMSForwarder::send(const StringRef& msg) {
    MessageEx::sms(inttostr(GETINT(cfg::sms::number)), msg, GETSTR(cfg::sms::gate), GETSTR(cfg::sms::sig));
  }

  void SMSForwarder::onISetCols() {
    Ctrl->SetColumn(tableConfig, cfg::tpl::smsForward, ctypeString, 
      "Wiadomo�� od kontaktu [{msgFrom}] wys�ana o {msgTime}{, msgDate}:"
      "\r\n\r\n{msgBody}", "kAway2/tpl/smsForward");

    Ctrl->SetColumn(tableConfig, cfg::tpl::smsSummary, ctypeString, 
      "Od czasu w��czenia away'a, dosta�e� {msgCount} wiadomo�ci od {userCount} os�b ({userList}).\r\n"
      "Ostatnia otrzymana wiadomo�� jest od {lastMsgFrom}.", "kAway2/tpl/smsSummary");

    Ctrl->SetColumn(tableConfig, cfg::sms::gate, ctypeString, "", "kAway2/sms/gate");
    Ctrl->SetColumn(tableConfig, cfg::sms::number, ctypeInt, 0, "kAway2/sms/number");
    Ctrl->SetColumn(tableConfig, cfg::sms::sig, ctypeString, "kAway2", "kAway2/sms/sig");

    Forwarder::onISetCols();
  }

  void SMSForwarder::onIPrepare() {
    UIActionCfgAdd(ui::sms::cfgGroup, 0, ACTT_GROUP, "Ustawienia");
    UIActionCfgAdd(ui::sms::cfgGroup, ui::sms::number, ACTT_EDIT | ACTSC_INLINE | ACTSC_INT | ACTR_CHECK | ACTR_INIT, 0, 
      cfg::sms::number, 0, 0, 120);
    UIActionCfgAdd(ui::sms::cfgGroup, 0, ACTT_COMMENT, "Numer na kt�ry b�d� wysy�ane SMSy");
    UIActionCfgAdd(ui::sms::cfgGroup, 0, ACTT_EDIT | ACTSC_INLINE, AP_TIPRICH 
      "<b>Uwaga</b>: niekt�re bramki SMS wymagaj� podpisu do poprawnego wys�ania wiadomo�ci!", cfg::sms::sig, 0, 0, 120);
    UIActionCfgAdd(ui::sms::cfgGroup, 0, ACTT_COMMENT, "Podpis dodawany do SMSa");
    UIActionCfgAdd(ui::sms::cfgGroup, ui::sms::gates, ACTT_COMBO | ACTSC_INLINE | ACTSCOMBO_LIST | ACTSCOMBO_NOICON | ACTSCOMBO_SORT, 0,
      cfg::sms::gate, 0, 0, 120);
    UIActionCfgAdd(ui::sms::cfgGroup, 0, ACTT_COMMENT, "Bramka kt�ra zostanie u�yta do wys�ania wiadomo�ci");
    UIActionCfgAdd(ui::sms::cfgGroup, 0, ACTT_GROUPEND);

    Forwarder::onIPrepare();
  }

  void SMSForwarder::refreshCombo(ActionEvent& ev) {
    int code = ev.getCode();
    if (code == ACTN_CHECK || code == ACTN_CREATE) {
      UIActionSetText(ui::sms::cfgGroup, ui::sms::gates, (char*) Ctrl->IMessage(Sms::IM::getGatewaysComboText, NET_SMS, imtAll, (int) UIActionCfgGetValue(sUIAction(ui::sms::cfgGroup, ui::sms::number), 0, 0)));
    }
  }
}