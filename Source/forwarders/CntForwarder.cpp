/**
  *  Contact Forwarder class
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

#include "CntForwarder.h"
#include "../Controller.h"

namespace kAway2 {
  CntForwarder::CntForwarder() : Forwarder("forward", "Lustereczko", ico::forward, true, true) {
    IMessageDispatcher& dispatcher = Context::getInstance()->getIMessageDispatcher();

    dispatcher.connect(IM_SETCOLS, bind(resolve_cast0(&CntForwarder::onISetCols), this));
    dispatcher.connect(IM_UI_PREPARE, bind(resolve_cast0(&CntForwarder::onIPrepare), this));
    dispatcher.connect(IM_UIACTION, bind(&CntForwarder::onAction, this, _1));
    dispatcher.connect(IM_MSG_RCV, bind(&CntForwarder::onNewMsg, this, _1)); // baaad
    dispatcher.connect(im::away, bind(resolve_cast0(&CntForwarder::onEnable), this));
    dispatcher.connect(im::back, bind(resolve_cast0(&CntForwarder::onDisable), this));
  }

  void CntForwarder::send(const StringRef& msg) {
    String ext;
    ext = SetExtParam(ext, cfg::extParamName, inttostr(cfg::tpl::forward));
    ext = SetExtParam(ext, MEX_ADDINFO, "kAway2");
    ext = SetExtParam(ext, MEX_NOSOUND, "1");

    if (GETINT(cfg::forward::type) == toCnt) {
      MessageEx::send(GETINT(cfg::forward::cnt), msg, Message::typeMessage, ext, true);
    } else {
      MessageEx::send(GETSTR(cfg::forward::uid), GETINT(cfg::forward::net), msg, Message::typeMessage, ext, true);
    }
  }

  void CntForwarder::onNewMsg(IMEvent& ev) {
    // TODO: wykrywanie zap�tlania
    Forwarder::onNewMsg(ev);
  }

  void CntForwarder::onISetCols() {
    Ctrl->SetColumn(tableConfig, cfg::tpl::forward, ctypeString, 
      "Wiadomo�� od kontaktu [<b>{msgFrom}</b>] wys�ana o <b>{msgTime}</b>{, msgDate}:"
      "\r\n\r\n{msgBody}", "kAway2/tpl/forward");

    Ctrl->SetColumn(tableConfig, cfg::tpl::forwardSummary, ctypeString, 
      "Od czasu w��czenia away'a, dosta�e� <b>{msgCount}</b> wiadomo�ci od <b>{userCount}</b> os�b ({userList}).\r\n"
      "Ostatnia otrzymana wiadomo�� jest od <b>{lastMsgFrom}</b>.", "kAway2/tpl/forwardSummary");

    Ctrl->SetColumn(tableConfig, cfg::forward::cnt, ctypeInt, 0, "kAway2/forward/cnt");
    Ctrl->SetColumn(tableConfig, cfg::forward::type, ctypeInt, toCnt, "kAway2/forward/type");
    Ctrl->SetColumn(tableConfig, cfg::forward::uid, ctypeString, "", "kAway2/forward/uid");
    Ctrl->SetColumn(tableConfig, cfg::forward::net, ctypeInt, 0, "kAway2/forward/net");

    Forwarder::onISetCols();
  }

  void CntForwarder::onIPrepare() {
    IconRegister(IML_16, ico::forward, Ctrl->hDll(), IDI_FORWARD);

    UIActionCfgAdd(ui::forward::cfgGroup, 0, ACTT_GROUP, "Wiadomo�ci przesy�aj ...");
    UIActionCfgAdd(ui::forward::cfgGroup, 0, ACTT_RADIO | ACTSRADIO_BYPOS, "... do podanego kontaktu", cfg::forward::type);
    UIActionCfgAdd(ui::forward::cfgGroup, ui::forward::userCombo, ACTT_COMBO | ACTSCOMBO_LIST | ACTR_INIT, 0, 
      cfg::forward::cnt);

    UIActionCfgAdd(ui::forward::cfgGroup, 0, ACTT_SEPARATOR);
    UIActionCfgAdd(ui::forward::cfgGroup, 0, ACTT_RADIO | ACTSRADIO_BYPOS | ACTSRADIO_LAST, "... na podany ni�ej adres", 
      cfg::forward::type);
    UIActionCfgAdd(ui::forward::cfgGroup, ui::forward::netsCombo, ACTT_COMBO | ACTSCOMBO_LIST | ACTR_INIT | ACTSC_INLINE, 0, 
      cfg::forward::net);
    UIActionCfgAdd(ui::forward::cfgGroup, 0, ACTT_EDIT | ACTSC_FULLWIDTH, 0, cfg::forward::uid);
    UIActionCfgAdd(ui::forward::cfgGroup, 0, ACTT_GROUPEND);

    Forwarder::onIPrepare();
  }

  void CntForwarder::onAction(IMEvent& ev) {
    ActionEvent aev(ev.getIMessage(), Controller::getInstance()->getActionDispatcher());
    int id = aev.getID(), code = aev.getCode();

    if (id == ui::forward::userCombo && code == ACTN_CREATE) {
      int count = Ctrl->IMessage(IMC_CNT_COUNT);
      String combo;

      for (int i = 0; i < count; i++) {
        if (i) {
          combo += strlen(GETCNTC(i, CNT_DISPLAY)) ? GETCNTC(i, CNT_DISPLAY) : GETCNTC(i, CNT_UID);
          combo += AP_ICO + inttostr(UIIcon(IT_LOGO, GETCNTI(i, CNT_NET), 0, 0));
          combo += AP_VALUE + inttostr(i) + "\n";
        } else {
          combo += "Wy��czone" AP_ICO "#2E" AP_VALUE "0\n";
        }
      }
      UIActionSetText(ui::forward::cfgGroup, ui::forward::userCombo, Helpers::trim(combo).c_str());
    } else if (id == ui::forward::netsCombo && code == ACTN_CREATE) {
      String combo, name;

      int count = Ctrl->IMessage(IMC_PLUG_COUNT);
      int id, type, net;

      for (int i = 0; i < count; i++) {
        if (i) {
          id = Ctrl->IMessage(IMC_PLUG_ID, 0, 0, i);
          type = Ctrl->IMessageDirect(IM_PLUG_TYPE, id);

          if ((type & imtNet) == imtNet) {
            net = (int) Ctrl->IMessageDirect(IM_PLUG_NET, id);
            name = (char*) Ctrl->IMessageDirect(IM_PLUG_NETNAME, id);

            if (name.length()) {
              combo += name + AP_ICO + inttostr(UIIcon(IT_LOGO, net, 0, 0));
              combo += AP_VALUE + inttostr(net) + "\n";
            }
          }
        } else {
          combo += "Brak" AP_ICO "#2E" AP_VALUE "0\n";
        }
      }
      UIActionSetText(ui::forward::cfgGroup, ui::forward::netsCombo, Helpers::trim(combo).c_str());
    }
  }
}