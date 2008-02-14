/**
  *  kAway2 Controller class
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
#include "Controller.h"

#include "Forwarders/CntForwarder.h"
#include "Forwarders/SMSForwarder.h"

namespace kAway2 {
  Controller::Controller() : _active(false), muteStateSwitched(false), autoAway(typeDisabled), _plugins_group(0) {
    // get dispatchers
    IMessageDispatcher& dispatcher = getIMessageDispatcher();
    ActionDispatcher& action_dispatcher = getActionDispatcher();

    /* Static values like net, type or version */
    dispatcher.setStaticValue(IM_PLUG_TYPE, IMT_UI | IMT_CONFIG | IMT_ALLMESSAGES);
    dispatcher.setStaticValue(IM_PLUG_PRIORITY, PLUGP_HIGHEST);
    dispatcher.setStaticValue(IM_PLUG_NAME, (int) "kAway2");
    dispatcher.setStaticValue(IM_PLUG_SIG, (int) sig);
    dispatcher.setStaticValue(IM_PLUG_NET, net);

    /* Callbacks */
    dispatcher.connect(IM_UI_PREPARE, bind(&Controller::_prepare, this, _1));
    dispatcher.connect(IM_UI_PREPARE, bind(&Controller::_prepareUI, this, _1));

    dispatcher.connect(IM_MSG_RCV, bind(&Controller::onMsgRcv, this, _1));
    dispatcher.connect(IM_BEFOREEND, bind(&Controller::onEnd, this, _1));
    dispatcher.connect(IM_ALLPLUGINSINITIALIZED, bind(&Controller::onPluginsLoaded, this, _1));
    dispatcher.connect(IM_AWAY, bind(&Controller::onAutoAway, this, _1));
    dispatcher.connect(IM_BACK, bind(&Controller::onBack, this, _1));

    /* API callbacks */
    dispatcher.connect(api::isEnabled, bind(&Controller::apiEnabled, this, _1));
    dispatcher.connect(api::enable, bind(&Controller::apiEnable, this, _1));
    dispatcher.connect(api::disable, bind(&Controller::apiDisable, this, _1));
    dispatcher.connect(api::isIgnored, bind(&Controller::apiIgnored, this, _1));
    dispatcher.connect(api::isAutoAway, bind(&Controller::apiAutoAway, this, _1));
    dispatcher.connect(api::ignore, bind(&Controller::apiIgnore, this, _1));
    dispatcher.connect(api::showAwayWnd, bind(&Controller::apiShowAwayWnd, this, _1));

    // actions
    action_dispatcher.connect(ui::cntCfgGroup, bind(&Controller::_handleCntGroup, this, _1));
    action_dispatcher.connect(ui::msgTbGrp, bind(&Controller::_handleMsgTb, this, _1));
    action_dispatcher.connect(ui::powerInMainWnd, bind(&Controller::_handlePowerBtns, this, _1));
    action_dispatcher.connect(ui::powerInCntWnd, bind(&Controller::_handlePowerBtns, this, _1));
    action_dispatcher.connect(ui::powerInTrayMenu, bind(&Controller::_handlePowerBtns, this, _1));
    action_dispatcher.connect(ui::ignoreBtn, bind(&Controller::_handleIgnoreBtn, this, _1));
    action_dispatcher.connect(act::clearMru, bind(&Controller::_clearMRU, this, _1));
    action_dispatcher.connect(act::resetCntSettings, bind(&Controller::_resetContactSettings, this, _1));
    action_dispatcher.connect(act::resetSettings, bind(&Controller::_resetGlobalSettings, this, _1));

    // bind extended away timer
    extAutoAwayTimer.reset(timerTmplCreate(bind(&Controller::onExtAutoAway, this)));

    // get instance of Config object
    Config& config = getConfig();

    /* Configuration columns */
    config.setColumn(DTCFG, cfg::autoAwaySync, DT_CT_INT, syncExtended, "kAway2/autoAwaySync");
    config.setColumn(DTCFG, cfg::useKNotify, DT_CT_INT, 1, "kAway2/useKNotify");
    config.setColumn(DTCFG, cfg::ircCmds, DT_CT_INT, 1, "kAway2/ircCmds");
    config.setColumn(DTCFG, cfg::disableOnMsgSend, DT_CT_INT, 0, "kAway2/disableOnMsgSend");
    config.setColumn(DTCFG, cfg::saveToHistory, DT_CT_INT, 1, "kAway2/saveToHistory");
    config.setColumn(DTCFG, cfg::mruSize, DT_CT_INT, 20, "kAway2/mruSize");
    config.setColumn(DTCFG, cfg::muteOnEnable, DT_CT_INT, 0, "kAway2/muteOnEnable");
    config.setColumn(DTCFG, cfg::confirmation, DT_CT_INT, 0, "kAway2/confirmation");
    config.setColumn(DTCFG, cfg::autoAwayMsg, DT_CT_STR, "auto-away", "kAway2/autoAwayMsg");

    config.setColumn(DTCFG, cfg::btnInMainWindow, DT_CT_INT, 1, "kAway2/btnInMainWindow");
    config.setColumn(DTCFG, cfg::btnInCntWindow, DT_CT_INT, 1, "kAway2/btnInCntWindow");
    config.setColumn(DTCFG, cfg::btnInTrayMenu, DT_CT_INT, 1, "kAway2/btnInTrayMenu");

    config.setColumn(DTCFG, cfg::dateFormat, DT_CT_STR, "%d/%m/%Y", "kAway2/dateFormat");
    config.setColumn(DTCFG, cfg::timeFormat, DT_CT_STR, "%H:%M", "kAway2/timeFormat");

    config.setColumn(DTCFG, cfg::wnd::changeInfoOnEnable, DT_CT_INT, 1, "kAway2/wnd/changeInfoOnEnable");
    config.setColumn(DTCFG, cfg::wnd::changeOnEnable, DT_CT_INT, 1, "kAway2/wnd/changeOnEnable");
    config.setColumn(DTCFG, cfg::wnd::muteOnEnable, DT_CT_INT, 0, "kAway2/wnd/muteOnEnable");
    config.setColumn(DTCFG, cfg::wnd::onEnableSt, DT_CT_INT, ST_NA, "kAway2/wnd/onEnableSt");

    config.setColumn(DTCFG, cfg::summary::interval, DT_CT_INT, 60, "kAway2/summary/interval");
    config.setColumn(DTCFG, cfg::summary::inAutoAway, DT_CT_INT, 0, "kAway2/summary/inAutoAway");
    config.setColumn(DTCFG, cfg::summary::minMsgCount, DT_CT_INT, 0, "kAway2/summary/minMsgCount");

    config.setColumn(DTCFG, cfg::fwd::inAutoAway, DT_CT_INT, 0, "kAway2/fwd/inAutoAway");

    config.setColumn(DTCFG, cfg::tpl::enable, DT_CT_STR, "brb/afk {[msg]}", "kAway2/tpl/enable");
    config.setColumn(DTCFG, cfg::tpl::disable, DT_CT_STR, "i'm back {[msg] }:>", "kAway2/tpl/disable");
    config.setColumn(DTCFG, cfg::tpl::reply, DT_CT_STR, 
      "Hello <b>{display|uid}</b>, i'm away from {date, }<b>{time}</b> {[msg]}.\r\n"
      "Leave a message after the beep. Byeee.", "kAway2/tpl/reply");
    config.setColumn(DTCFG, cfg::tpl::status, DT_CT_STR, "{status |} {{msg} }{[time]}", "kAway2/tpl/status");

    config.setColumn(DTCFG, cfg::reply::onEnable, DT_CT_INT, 0, "kAway2/reply/onEnable");
    config.setColumn(DTCFG, cfg::reply::onDisable, DT_CT_INT, 0, "kAway2/reply/onDisable");
    config.setColumn(DTCFG, cfg::reply::onMsg, DT_CT_INT, 1, "kAway2/reply/onMsg");
    config.setColumn(DTCFG, cfg::reply::whenInvisible, DT_CT_INT, 0, "kAway2/reply/whenInvisible");
    config.setColumn(DTCFG, cfg::reply::showInWnd, DT_CT_INT, 1, "kAway2/reply/showInWnd");
    config.setColumn(DTCFG, cfg::reply::minInterval, DT_CT_INT, 900, "kAway2/reply/minInterval");
    config.setColumn(DTCFG, cfg::reply::minIntervalType, DT_CT_INT, typeBoth, "kAway2/reply/minIntervalType");
    config.setColumn(DTCFG, cfg::reply::useHtml, DT_CT_INT, 1, "kAway2/reply/useHtml");
    config.setColumn(DTCFG, cfg::reply::netChange, DT_CT_STR, "", "kAway2/reply/netChange");

    config.setColumn(DTCFG, cfg::status::onEnableSt, DT_CT_INT, ST_NA, "kAway2/status/onEnableSt");
    config.setColumn(DTCFG, cfg::status::onAutoAwaySt, DT_CT_INT, ST_AWAY, "kAway2/status/onAutoAwaySt");
    config.setColumn(DTCFG, cfg::status::whenInvisible, DT_CT_INT, 0, "kAway2/status/whenInvisible");
    config.setColumn(DTCFG, cfg::status::changeOnEnable, DT_CT_INT, 1, "kAway2/status/changeOnEnable");
    config.setColumn(DTCFG, cfg::status::changeInfoOnEnable, DT_CT_INT, 1, "kAway2/status/changeInfoOnEnable");
    config.setColumn(DTCFG, cfg::status::netChange, DT_CT_STR, "", "kAway2/status/netChange");
    config.setColumn(DTCFG, cfg::status::dotsAppend, DT_CT_INT, 1, "kAway2/status/dotsAppend");
    config.setColumn(DTCFG, cfg::status::chgOnlyIfOnline, DT_CT_INT, 1, "kAway2/status/chgOnlyIfOnline");

    config.setColumn(DTCFG, cfg::extAutoAway::status, DT_CT_INT, ST_NA, "kAway2/extAutoAway/status");
    config.setColumn(DTCFG, cfg::extAutoAway::time, DT_CT_INT, 1800, "kAway2/extAutoAway/time");

    config.setColumn(DTCNT, cfg::tpl::enable, DT_CT_STR, "", "kAway2/tpl/enable");
    config.setColumn(DTCNT, cfg::tpl::disable, DT_CT_STR, "", "kAway2/tpl/disable");
    config.setColumn(DTCNT, cfg::tpl::reply, DT_CT_STR, "", "kAway2/tpl/reply");

    config.setColumn(DTCNT, cfg::saveToHistory, DT_CT_INT, 0, "kAway2/saveToHistory");
    config.setColumn(DTCNT, cfg::disableOnMsgSend, DT_CT_INT, 0, "kAway2/disableOnMsgSend");
    config.setColumn(DTCNT, cfg::ircCmds, DT_CT_INT, 0, "kAway2/ircCmds");

    config.setColumn(DTCNT, cfg::reply::onEnable, DT_CT_INT, 0, "kAway2/reply/onEnable");
    config.setColumn(DTCNT, cfg::reply::onDisable, DT_CT_INT, 0, "kAway2/reply/onDisable");
    config.setColumn(DTCNT, cfg::reply::onMsg, DT_CT_INT, 0, "kAway2/reply/onMsg");
    config.setColumn(DTCNT, cfg::reply::whenInvisible, DT_CT_INT, 0, "kAway2/reply/whenInvisible");
    config.setColumn(DTCNT, cfg::reply::showInWnd, DT_CT_INT, 0, "kAway2/reply/showInWnd");
    config.setColumn(DTCNT, cfg::reply::minInterval, DT_CT_INT, -1, "kAway2/reply/minInterval");
    config.setColumn(DTCNT, cfg::reply::minIntervalType, DT_CT_INT, -1, "kAway2/reply/minIntervalType");
    config.setColumn(DTCNT, cfg::reply::useHtml, DT_CT_INT, 0, "kAway2/reply/useHtml");
  }

  /* IMessage callback methods */

  void Controller::_prepare(IMEvent& ev) {
    // get ActionDispatcher object
    ActionDispatcher& action_dispatcher = getActionDispatcher();

    // set plugins group id
    this->_plugins_group = Helpers::getPluginsGroup();

    // set up MRU list and away window
    this->mruList = new MRUConfigurable(cfg::mruName, cfg::mruSize);
    this->wnd = new AwayWnd();

    this->statusList = new NetList(action_dispatcher, cfg::status::netChange, ui::statusCfgGroup, dynAct::status, 
      act::cfgGroupCheckCreate);
    statusList->addIgnored(plugsNET::konnferencja);
    statusList->addIgnored(plugsNET::klan);
    statusList->addIgnored(plugsNET::checky);
    statusList->addIgnored(plugsNET::actio);
    statusList->load();

    this->autoReplyList = new NetList(action_dispatcher, cfg::reply::netChange, ui::replyCfgGroup, dynAct::reply, 
      act::replyCfgGroupCheckCreate);
    autoReplyList->addIgnored(plugsNET::klan);
    autoReplyList->addIgnored(plugsNET::checky);
    autoReplyList->addIgnored(plugsNET::actio);
    autoReplyList->load();

    this->statusCtrl = new FormattedStatus(statusList, cfg::status::whenInvisible, cfg::status::dotsAppend, "status");
    statusCtrl->addReplacementSt(plugsNET::gg, ST_CHAT, ST_ONLINE);
    statusCtrl->addReplacementSt(plugsNET::gg, ST_DND, ST_AWAY);
    statusCtrl->addReplacementSt(plugsNET::gg, ST_NA, ST_AWAY);

    // fwdCtrl->fwdRegister(new SMSForwarder);
    // fwdCtrl->fwdRegister(new CntForwarder);

    // connect status listener
    getIMessageDispatcher().connect(IM_STATUSCHANGE, bind(&Status::actionHandle, statusCtrl, _1));

    log("[Controller::prepare()]: Ctrl = %i, sCtrl = %i, wnd = %i", 
      Ctrl, statusCtrl, wnd);
  }

  void Controller::_prepareUI(IMEvent& ev) {
    /* Defining help variables */
    Format::tHelpVars stVars, rVars;

    stVars.push_back(Format::sHelpVar("status", "Aktualny opis statusu"));
    stVars.push_back(Format::sHelpVar("msg", "Przyczyna nieobecno�ci"));
    stVars.push_back(Format::sHelpVar("date", "Data w��czenia trybu away"));
    stVars.push_back(Format::sHelpVar("time", "Czas w��czenia trybu away"));

    rVars.push_back(Format::sHelpVar("uid", "Identyfikator kontaktu w sieci"));
    rVars.push_back(Format::sHelpVar("display", "Nazwa wy�wietlania kontaktu"));
    rVars.push_back(Format::sHelpVar("name", "Imi� przypisane do kontaktu"));
    rVars.push_back(Format::sHelpVar("nick", "Ksywka przypisana do kontaktu"));
    rVars.push_back(Format::sHelpVar("surname", "Nazwisko przypisane do kontaktu"));
    rVars.push_back(Format::sHelpVar("date", "Data w��czenia trybu away"));
    rVars.push_back(Format::sHelpVar("time", "Czas w��czenia trybu away"));
    rVars.push_back(Format::sHelpVar("msg", "Przyczyna nieobecno�ci/powrotu"));

    /* Registering icons */
    IconRegister(IML_32, ico::logoBig, Ctrl->hDll(), IDI_LOGO);
    // IconRegister(IML_16, ico::logoSmall, Ctrl->hDll(), IDI_LOGO);
    IconRegister(IML_16, ico::logoSmall, Ctrl->hDll(), IDI_ENABLE);

    IconRegister(IML_16, ico::enable, Ctrl->hDll(), IDI_ENABLE);
    IconRegister(IML_16, ico::disable, Ctrl->hDll(), IDI_DISABLE);

    IconRegister(IML_16, ico::ignore, Ctrl->hDll(), IDI_IGNORE);
    IconRegister(IML_16, ico::unIgnore, Ctrl->hDll(), IDI_UNIGNORE);

    IconRegister(IML_16, ico::status, Ctrl->hDll(), IDI_STATUS);
    IconRegister(IML_16, ico::reply, Ctrl->hDll(), IDI_REPLY);

    IconRegister(IML_16, ico::accept, Ctrl->hDll(), IDI_ACCEPT);
    IconRegister(IML_16, ico::cancel, Ctrl->hDll(), IDI_CANCEL);

    // IconRegister(IML_16, ico::help, Ctrl->hDll(), IDI_HELP);
    IconRegister(IML_16, ico::trash, Ctrl->hDll(), IDI_TRASH);
    IconRegister(IML_16, ico::msg, Ctrl->hDll(), IDI_MSG);

    /* Adding configuration tabs */
    UIGroupAdd(IMIG_NFO, ui::cntCfgGroup, ACTR_SAVE | ACTR_INIT, "kAway2", ico::logoSmall);
    UIGroupAdd(IMIG_CFG_PLUGS, ui::cfgGroup, ACTR_SAVE, "kAway2", ico::logoSmall);
    UIGroupAdd(ui::cfgGroup, ui::replyCfgGroup, ACTR_SAVE, "Autoresponder", ico::reply);
    UIGroupAdd(ui::cfgGroup, ui::statusCfgGroup, ACTR_SAVE, "Opcje statusu", ico::status);

    /* Plugin info box */
    char header[400];
    sprintf_s<400>(header, "<span class='note'>Powered by: <b>%s</b></span><br/>"
      "<span class='note'>Skompilowano: <b>%s</b> [<b>%s</b>]</span><br/>"
      "Informacje o wtyczce na forum Konnekta "
      "(http://konnekt.info/forum/)<br/><br/>"
      "Copyright � 2004-2008 <b>Sijawusz Pur Rahnama</b><br/>"
      "Copyright � 2004-2008 <b>KPlugins Team</b>",
      poweredBy, __DATE__, __TIME__);

    char desc[] = 
      "Wtyczka ma za zadanie zaj�� si� kwesti� Twojej nieobecno�ci przy komputerze :)<br/>"
      "Osoby pisz�ce do Ciebie zostan� powiadomione (albo i nie ;)) o tym od kiedy i dlaczego Cie nie ma, "
      "ty za to dostaniesz informacj� o tym ile os�b nawiedzi�o Ci� podczas Twoich godzin-bez-komputera ;>";

    char dateFormat[] = AP_TIPRICH
      "%<b>a</b> - Abbreviated weekday name<br/>"
      "%<b>A</b> - Full weekday name<br/>"
      "%<b>b</b> - Abbreviated month name<br/>"
      "%<b>B</b> - Full month name<br/>"
      "%<b>d</b> - Day of month as decimal number (01 � 31)<br/>"
      "%<b>j</b> - Day of year as decimal number (001 � 366)<br/>"
      "%<b>m</b> - Month as decimal number (01 � 12)<br/>"
      "%<b>U</b> - Week of year as decimal number, with Sunday as first day of week (00 � 53)<br/>"
      "%<b>w</b> - Weekday as decimal number (0 � 6; Sunday is 0)<br/>"
      "%<b>W</b> - Week of year as decimal number, with Monday as first day of week (00 � 53)<br/>"
      "%<b>x</b> - Date representation for current locale<br/>"
      "%<b>y</b> - Year without century, as decimal number (00 � 99)<br/>"
      "%<b>Y</b> - Year with century, as decimal number<br/>"
      "%<b>z</b>, %<b>Z</b> - Either the time-zone name or time zone abbreviation, depending on registry settings; "
      "no characters if time zone is unknown<br/>"
      "%<b>%</b> - Percent sign"
      AP_TIP_WIDTH "300";

    char timeFormat[] = AP_TIPRICH
      "%<b>H</b> - Hour in 24-hour format (00 � 23)<br/>"
      "%<b>I</b> - Hour in 12-hour format (01 � 12)<br/>"
      "%<b>M</b> - Minute as decimal number (00 � 59)<br/>"
      "%<b>p</b> - Current locale's A.M./P.M. indicator for 12-hour clock<br/>"
      "%<b>S</b> - Second as decimal number (00 � 59)<br/>"
      "%<b>X</b> - Time representation for current locale<br/>"
      "%<b>%</b> - Percent sign"
      AP_TIP_WIDTH "285";

    UIActionCfgAddPluginInfoBox2(ui::cfgGroup, desc, header, Helpers::icon16(ico::logoSmall).a_str(), -4);
    UIActionCfgAddPluginInfoBox2(ui::cntCfgGroup, desc, header, Helpers::icon16(ico::logoSmall).a_str(), -4);

    /* User tab */
    ifINT {
      UIActionCfgAdd(43, 0, ACTT_GROUP, "Tryb auto-away (podstawowy)");
      UIActionCfgAdd(43, 0, ACTT_SEPARATOR, "Zmieniaj status na:");
      UIActionCfgAdd(43, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(UIIcon(IT_STATUS, 0, ST_AWAY, 0)).a_str());
      UIActionCfgAdd(43, 0, ACTT_RADIO | ACTSC_INLINE, "Zaraz wracam" AP_VALUE "65", cfg::status::onAutoAwaySt);
      UIActionCfgAdd(43, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(UIIcon(IT_STATUS, 0, ST_NA, 0)).a_str());
      UIActionCfgAdd(43, 0, ACTT_RADIO | ACTSC_INLINE, "Nieosi�galny" AP_VALUE "33", cfg::status::onAutoAwaySt);
      UIActionCfgAdd(43, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(UIIcon(IT_STATUS, 0, ST_DND, 0)).a_str());
      UIActionCfgAdd(43, 0, ACTT_RADIO | ACTSC_INLINE, "Nie przeszkadza�" AP_VALUE "34", cfg::status::onAutoAwaySt);
      UIActionCfgAdd(43, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(UIIcon(IT_STATUS, 0, ST_HIDDEN, 0)).a_str());
      UIActionCfgAdd(43, 0, ACTT_RADIO | ACTSRADIO_LAST, "Ukryty" AP_VALUE "66", cfg::status::onAutoAwaySt);
      UIActionCfgAdd(43, 0, ACTT_SEPARATOR);
      UIActionCfgAdd(43, 0, ACTT_CHECK, "Tylko przy statusach 'dost�pny' i 'pogadam'", cfg::status::chgOnlyIfOnline);
      UIActionCfgAdd(43, 0, ACTT_GROUPEND);

      UIActionCfgAdd(43, 0, ACTT_GROUP, "Tryb auto-away (rozszerzony)");
      UIActionCfgAdd(43, 0, ACTT_SEPARATOR, "Po jakim czasie zmienia� status:");
      UIActionCfgAdd(43, 0, ACTT_SLIDER, "15m, 1s\n45m" AP_STEP "60" AP_MINIMUM "901" AP_MAXIMUM "2700"
        AP_TIP "Czas w sekundach po jakim zostanie ustawiony wybrany poni�ej status", cfg::extAutoAway::time);
      UIActionCfgAdd(43, 0, ACTT_SEPARATOR, "Zmieniaj status na:");
      UIActionCfgAdd(43, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(UIIcon(IT_STATUS, 0, ST_AWAY, 0)).a_str());
      UIActionCfgAdd(43, 0, ACTT_RADIO | ACTSC_INLINE, "Zaraz wracam" AP_VALUE "65", cfg::extAutoAway::status);
      UIActionCfgAdd(43, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(UIIcon(IT_STATUS, 0, ST_NA, 0)).a_str());
      UIActionCfgAdd(43, 0, ACTT_RADIO | ACTSC_INLINE, "Nieosi�galny" AP_VALUE "33", cfg::extAutoAway::status);
      UIActionCfgAdd(43, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(UIIcon(IT_STATUS, 0, ST_DND, 0)).a_str());
      UIActionCfgAdd(43, 0, ACTT_RADIO | ACTSC_INLINE, "Nie przeszkadza�" AP_VALUE "34", cfg::extAutoAway::status);
      UIActionCfgAdd(43, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(UIIcon(IT_STATUS, 0, ST_HIDDEN, 0)).a_str());
      UIActionCfgAdd(43, 0, ACTT_RADIO, "Ukryty" AP_VALUE "66", cfg::extAutoAway::status);
      UIActionCfgAdd(43, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(46).a_str());
      UIActionCfgAdd(43, 0, ACTT_RADIO | ACTSC_BOLD | ACTSRADIO_LAST, "Nie zmieniaj" AP_VALUE "-1", cfg::extAutoAway::status);
      UIActionCfgAdd(43, 0, ACTT_GROUPEND);

      UIActionCfgAdd(43, 0, ACTT_GROUP, "Szablon statusu");
      Format::UIDrawHelpBtn(stVars, 43);
      UIActionCfgAdd(43, 0, ACTT_TEXT, 0, cfg::tpl::status);
      UIActionCfgAdd(43, 0, ACTT_GROUPEND);
    }

    /* Main tab */
    /* |-> General settings group */
    ifINT {
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUP, "Ustawienia");
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_CHECK, "Wycisz przy w��czeniu trybu away", cfg::muteOnEnable);
      ifADV UIActionCfgAdd(ui::cfgGroup, 0, ACTT_CHECK, "Wy��czaj w momencie wys�ania wiadomo�ci", cfg::disableOnMsgSend);
      ifADV UIActionCfgAdd(ui::cfgGroup, 0, ACTT_CHECK, "Zapisuj wiadomo�ci w osobnej historii", cfg::saveToHistory);
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_CHECK, "Komendy z okna rozm�w (a'la IRC)" AP_TIPRICH 
        "/<b>away</b> [...] - w��cza tryb away<br/>"
        "/<b>brb</b> [...] - j.w. + nie wysy�a powiadomie� o w�.<br/>"
        "/<b>back</b> [...] - wy��cza tryb away<br/>"
        "/<b>re</b> [...] - j.w. + nie wysy�a powiadomie� o wy�." AP_TIP_WIDTH "240", cfg::ircCmds);
      if (Helpers::pluginExists(plugsNET::knotify))
        UIActionCfgAdd(ui::cfgGroup, 0, ACTT_CHECK, "Pokazuj powiadomienia K.Notify", cfg::useKNotify);
      ifADV UIActionCfgAdd(ui::cfgGroup, 0, ACTT_CHECK, "Pytaj przed wyj�ciem z trybu away", cfg::confirmation);
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUPEND);
    }

    ifADV {
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUP, "Formatowanie czasu");
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_EDIT | ACTSC_INLINE, dateFormat, cfg::dateFormat);
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_COMMENT, "Format daty");
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_EDIT | ACTSC_INLINE, timeFormat, cfg::timeFormat);
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_COMMENT, "Format godziny");
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUPEND);
    }

    ifINT {
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUP, "Ustawienia auto-away'a");
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_SEPARATOR, "Synchronizuj z trybem auto-away:");
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_RADIO | ACTSC_INLINE, "podstawowym" AP_VALUE "1", cfg::autoAwaySync);
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_RADIO, "rozszerzonym" AP_VALUE "2", cfg::autoAwaySync);
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_RADIO | ACTSC_BOLD | ACTSRADIO_LAST, "nie synchronizuj" AP_VALUE "0", cfg::autoAwaySync);
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_SEPARATOR, "Pow�d nieobecno�ci:");
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_TEXT, 0, cfg::autoAwayMsg);
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUPEND);
    }

    /* |-> Interface group */
    UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUP, "Interfejs");
    UIActionCfgAdd(ui::cfgGroup, 0, ACTT_CHECK | ACTSC_NEEDRESTART, "Pokazuj przycisk w g��wnym oknie", cfg::btnInMainWindow);
    UIActionCfgAdd(ui::cfgGroup, 0, ACTT_CHECK | ACTSC_NEEDRESTART, "Pokazuj przycisk w oknie rozmowy", cfg::btnInCntWindow);
    UIActionCfgAdd(ui::cfgGroup, 0, ACTT_CHECK | ACTSC_NEEDRESTART, "Pokazuj przycisk w menu tray'a", cfg::btnInTrayMenu);
    UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUPEND);

    UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUP, "Historia powod�w nieobecno�ci");
    UIActionCfgAdd(ui::cfgGroup, 0, ACTT_SEPARATOR, "Ilo�� pozycji w historii:");
    UIActionCfgAdd(ui::cfgGroup, 0, ACTT_SLIDER, "Ma�o\nDu�o" AP_MINIMUM "1" AP_MAXIMUM "30", cfg::mruSize);
    UIActionCfgAdd(ui::cfgGroup, 0, ACTT_SEPARATOR);
    UIActionCfgAdd(ui::cfgGroup, act::clearMru, ACTT_BUTTON, "wyczy��" AP_ICO "667112", 0, 0, 0, 80, 30);
    UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUPEND);

    ifPRO {
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUP);
      UIActionCfgAdd(ui::cfgGroup, act::resetSettings, ACTT_BUTTON | ACTSC_INLINE, "resetuj ustawienia" AP_ICO "27", 0, 0, 0, 140, 30);
      UIActionCfgAdd(ui::cfgGroup, act::resetCntSettings, ACTT_BUTTON, "resetuj ustawienia kontakt�w" AP_ICO "27", 0, 0, 0, 180, 30);
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUPEND);
    }

    /*
    ifPRO {
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUP, "Ustawienia raportowania");
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_SEPARATOR, "Aktywne metody raportu:");

      //fCtrl->UIDrawActiveSum();

      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_SEPARATOR, "Odst�p czasu pomi�dzy wysy�anymi raportami [min]:");
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_SLIDER, "Nie wysy�aj\n1 dzie�" AP_MINIMUM "0" AP_MAXIMUM "1440" AP_STEP "60", 
        cfg::summary::interval);
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_SEPARATOR);
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_CHECK, "Wysy�aj w trybie auto-away", cfg::summary::inAutoAway);
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_SPINNER | ACTSC_INLINE, AP_MINIMUM "0" AP_MAXIMUM "1000" 
        AP_TIP "0 = bez limitu", cfg::summary::minMsgCount, 0, 0, 120);
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_COMMENT, "Minimalna ilo�� wiadomo�ci");
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUPEND);

      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUP, "Ustawienia przekierowania");
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_SEPARATOR, "Aktywne przekierowania:");

      //fCtrl->UIDrawActiveFwd();

      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_SEPARATOR);
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_CHECK, "Wysy�aj w trybie auto-away", cfg::fwd::inAutoAway);
      UIActionCfgAdd(ui::cfgGroup, 0, ACTT_GROUPEND);
    }
    */

    /* Status tab */
    /* |-> Settings group */
    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_GROUP, "Ustawienia");
    /*
    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_COMBO | ACTSCOMBO_LIST | ACTSC_INLINE, 
      "Zaraz wracam" AP_ICO "#40000410" AP_VALUE "65\n"
      "Nieosi�galny" AP_ICO "#40000210" AP_VALUE "33\n"
      "Nie przeszkadza�" AP_ICO "#40000220" AP_VALUE "34\n"
      "Ukryty" AP_ICO "#40000420" AP_VALUE "66", 
      cfg::status::onEnableSt);
    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_COMMENT, "Domy�lnie ustawiony status");
    */
    ifINT {
      ifNORM UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_CHECK, "Dodawaj '...' przy skracaniu d�ugich opis�w", cfg::status::dotsAppend);
      UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_CHECK, "Zmieniaj status przy w��czonym statusie 'ukryty'", cfg::status::whenInvisible);
      UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_SEPARATOR);
    }
    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_CHECK, "Zmieniaj status przy w��czeniu trybu away", cfg::status::changeOnEnable);
    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_CHECK, "Zmieniaj opis statusu przy w��czeniu trybu away", cfg::status::changeInfoOnEnable);
    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_GROUPEND);

    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_GROUP, "Domy�lnie zmieniaj status na:");
    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(UIIcon(IT_STATUS, 0, ST_AWAY, 0)).a_str());
    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_RADIO | ACTSC_INLINE, "Zaraz wracam" AP_VALUE "65", cfg::status::onEnableSt);
    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(UIIcon(IT_STATUS, 0, ST_NA, 0)).a_str());
    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_RADIO | ACTSC_INLINE, "Nieosi�galny" AP_VALUE "33", cfg::status::onEnableSt);
    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(UIIcon(IT_STATUS, 0, ST_DND, 0)).a_str());
    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_RADIO | ACTSC_INLINE, "Nie przeszkadza�" AP_VALUE "34", cfg::status::onEnableSt);
    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(UIIcon(IT_STATUS, 0, ST_HIDDEN, 0)).a_str());
    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_RADIO | ACTSRADIO_LAST, "Ukryty" AP_VALUE "66", cfg::status::onEnableSt);
    UIActionCfgAdd(ui::statusCfgGroup, 0, ACTT_GROUPEND);

    /* |-> Net selection group */
    statusList->drawGroup(3, "Wybierz sieci, na kt�rych chcesz zmienia� status:");

    /* Autoresponder tab */
    /* |-> Minimal reply interval group */
    ifPRO {
      UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_GROUP, "Minimalny interwa� pomi�dzy wysy�anymi odpowiedziami");
      UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_SEPARATOR, "Czas interwa�u:");
      UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_SLIDER, "Ma�y\nDu�y" AP_STEP "60" AP_MINIMUM "0" AP_MAXIMUM "3600" AP_TIPRICH 
        "<b>0</b> - odpowied� b�dzie wys�ana tylko jeden raz<br/><b>3600</b> - odpowied� nie zostanie ponownie "
        "wys�ana przez min. <b>1h</b> :>", cfg::reply::minInterval);
      UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_SEPARATOR, "Interwa� b�dzie liczony od ostatniej ...");
      UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(ico::msg).a_str());
      UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_RADIO | ACTSC_INLINE, "otrzymanej wiadomo�ci" AP_VALUE "0", cfg::reply::minIntervalType);
      UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(ico::reply).a_str());
      UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_RADIO, "wys�anej wiadomo�ci" AP_VALUE "1"
        AP_TIP "(w tym powiadomie�)", cfg::reply::minIntervalType);
      UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(ico::msg).a_str());
      UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_RADIO | ACTSRADIO_LAST, "wys�anej i otrzymanej wiadomo�ci" 
        AP_VALUE "2", cfg::reply::minIntervalType);
      UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_GROUPEND);
    }

    /* |-> Net selection group */
    autoReplyList->drawGroup(3, "Wybierz sieci, na kt�rych maj� dzia�a� powiadomienia:");

    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_GROUP, "Wysy�anie powiadomie�");
    ifINT {
      // ifADV UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_CHECK, "Pokazuj wys�ane powiadomienia w okienku rozmowy", cfg::reply::showInWnd);
      UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_CHECK, "Wysy�aj powiadomienia przy w��czonym statusie 'ukryty'", cfg::reply::whenInvisible);
      UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_SEPARATOR);
    }
    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_CHECK, "Odpowiadaj na odebrane wiadomo�ci", cfg::reply::onMsg);
    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_CHECK, "Wysy�aj powiadomienie o w��czeniu trybu away", cfg::reply::onEnable);
    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_CHECK, "Wysy�aj powiadomienie o wy��czeniu trybu away", cfg::reply::onDisable);
    /*
    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_SEPARATOR);
    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_CHECK, "Wy�lij powiadomienie je�li kontakt przy�le 'has�o'", cfg::reply::magicKey);
    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_COMBO | ACTSC_INLINE | ACTSCOMBO_LIST, 
      "SMS" AP_ICO "0x50000090" AP_VALUE "sms\n" 
      "EMail" AP_ICO "0x50000080" AP_VALUE "email\n" 
      "Przekierowanie" AP_ICO "0x50000080" AP_VALUE "forward", // bad icon for now
      cfg::reply::magicKeyNotifyMethod);
    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_COMMENT, "Spos�b wysy�ki");
    */
    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_GROUPEND);

    /* |-> Templates group */
    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_GROUP, "Szablony powiadomie�");
    ifINT UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_CHECK, "Formatuj kod HTML" AP_TIP 
      "Je�li w��czone, znaki specjalne nale�y zamienia� na ich HTML-owe odpowiedniki (np.: (< na &lt;), (> na &gt;))", cfg::reply::useHtml);

    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_SEPARATOR, "Odpowied�:");
    Format::UIDrawHelpBtn(rVars, ui::replyCfgGroup);
    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_TEXT, 0, cfg::tpl::reply);

    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_SEPARATOR, "W��czenie trybu away:");
    Format::UIDrawHelpBtn(rVars, ui::replyCfgGroup);
    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_TEXT, 0, cfg::tpl::enable);

    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_SEPARATOR, "Wy��czenie trybu away:");
    Format::UIDrawHelpBtn(rVars, ui::replyCfgGroup);
    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_TEXT, 0, cfg::tpl::disable);
    UIActionCfgAdd(ui::replyCfgGroup, 0, ACTT_GROUPEND);

    /* Main tab in contact window */
    /* |-> Minimal reply interval group */
    ifPRO {
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_GROUP, "Minimalny interwa� pomi�dzy wysy�anymi odpowiedziami");
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_SEPARATOR, "Czas interwa�u:");
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_SLIDER, "Ma�y\nDu�y" AP_STEP "60" AP_MINIMUM "-1" AP_MAXIMUM "3600" AP_TIPRICH 
        "<b>-1</b> - pobierze warto�� domy�ln� z konfiguracji<br/><b>0</b> - odpowied� b�dzie wys�ana tylko jeden raz<br/>"
        "<b>3600</b> - odpowied� nie zostanie ponownie wys�ana przez min. <b>1h</b> :>", cfg::reply::minInterval);
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_SEPARATOR, "Interwa� b�dzie liczony od ostatniej ...");
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(ico::msg).a_str());
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_RADIO | ACTSC_INLINE, "otrzymanej wiadomo�ci" AP_VALUE "0", cfg::reply::minIntervalType);
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(ico::reply).a_str());
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_RADIO, "wys�anej wiadomo�ci" AP_VALUE "1"
        AP_TIP "(w tym powiadomie�)", cfg::reply::minIntervalType);
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(ico::msg).a_str());
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_RADIO | ACTSC_INLINE, "wys�anej i otrzymanej wiadomo�ci" 
        AP_VALUE "2", cfg::reply::minIntervalType);
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_IMAGE | ACTSC_INLINE, Helpers::icon16(27).a_str());
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_RADIO | ACTSC_BOLD | ACTSRADIO_LAST, "domy�lnie" AP_VALUE "-1", cfg::reply::minIntervalType);
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_GROUPEND);
    }

    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_GROUP, "Ustawienia");
    ifADV {
      /* UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMBO | ACTSCOMBO_LIST | ACTSCOMBO_BYPOS | ACTSC_INLINE, 
        "Domy�lnie" CFGICO "#1B\nTak" CFGICO "#A2DE5\nNie" CFGICO "#A2DE6", cfg::reply::showInWnd);
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMMENT, "Pokazuj wys�ane powiadomienia w okienku rozmowy"); */

      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMBO | ACTSCOMBO_LIST | ACTSCOMBO_BYPOS | ACTSC_INLINE, 
        "Domy�lnie" CFGICO "#1B\nTak" CFGICO "#A2DE5\nNie" CFGICO "#A2DE6", cfg::saveToHistory);
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMMENT, "Zapisuj wiadomo�ci w osobnej historii");

      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMBO | ACTSCOMBO_LIST | ACTSCOMBO_BYPOS | ACTSC_INLINE, 
        "Domy�lnie" CFGICO "#1B\nTak" CFGICO "#A2DE5\nNie" CFGICO "#A2DE6", cfg::disableOnMsgSend);
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMMENT, "Wy��czaj w momencie wys�ania wiadomo�ci");
    }

    ifINT {
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMBO | ACTSCOMBO_LIST | ACTSCOMBO_BYPOS | ACTSC_INLINE, 
        "Domy�lnie" CFGICO "#1B\nTak" CFGICO "#A2DE5\nNie" CFGICO "#A2DE6", cfg::ircCmds);
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMMENT, "Komendy z okna rozm�w (a'la IRC)");
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_GROUPEND);

      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_GROUP, "Wysy�anie powiadomie�");
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMBO | ACTSCOMBO_LIST | ACTSCOMBO_BYPOS | ACTSC_INLINE, 
        "Domy�lnie" CFGICO "#1B\nTak" CFGICO "#A2DE5\nNie" CFGICO "#A2DE6", cfg::reply::whenInvisible);
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMMENT, "Wysy�aj powiadomienia przy w��czonym statusie 'ukryty'");
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_SEPARATOR);
    }

    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMBO | ACTSCOMBO_LIST | ACTSCOMBO_BYPOS | ACTSC_INLINE, 
      "Domy�lnie" CFGICO "#1B\nTak" CFGICO "#A2DE5\nNie" CFGICO "#A2DE6", cfg::reply::onMsg);
    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMMENT, "Odpowiadaj na odebrane wiadomo�ci");

    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMBO | ACTSCOMBO_LIST | ACTSCOMBO_BYPOS | ACTSC_INLINE, 
      "Domy�lnie" CFGICO "#1B\nTak" CFGICO "#A2DE5\nNie" CFGICO "#A2DE6", cfg::reply::onEnable);
    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMMENT, "Wysy�aj powiadomienie o w��czeniu trybu away");

    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMBO | ACTSCOMBO_LIST | ACTSCOMBO_BYPOS | ACTSC_INLINE, 
      "Domy�lnie" CFGICO "#1B\nTak" CFGICO "#A2DE5\nNie" CFGICO "#A2DE6", cfg::reply::onDisable);
    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMMENT, "Wysy�aj powiadomienie o wy��czeniu trybu away");
    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_GROUPEND);

    /* |-> Templates group */
    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_GROUP, "Szablony powiadomie�");
    ifINT {
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMBO | ACTSCOMBO_LIST | ACTSCOMBO_BYPOS | ACTSC_INLINE, 
        "Domy�lnie" CFGICO "#1B\nTak" CFGICO "#A2DE5\nNie" CFGICO "#A2DE6", cfg::reply::useHtml);
      UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_COMMENT, "Formatuj kod HTML" AP_TIP 
        "Je�li w��czone, znaki specjalne nale�y zamienia� na ich HTML-owe odpowiedniki (np.: (< na &lt;), (> na &gt;), etc.)");
    }

    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_SEPARATOR, "Odpowied�:");
    Format::UIDrawHelpBtn(rVars, ui::cntCfgGroup);
    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_TEXT, 0, cfg::tpl::reply);

    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_SEPARATOR, "W��czenie trybu away:");
    Format::UIDrawHelpBtn(rVars, ui::cntCfgGroup);
    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_TEXT, 0, cfg::tpl::enable);

    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_SEPARATOR, "Wy��czenie trybu away:");
    Format::UIDrawHelpBtn(rVars, ui::cntCfgGroup);
    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_TEXT, 0, cfg::tpl::disable);
    UIActionCfgAdd(ui::cntCfgGroup, 0, ACTT_GROUPEND);

    /* Buttons */
    if (Config::get(cfg::btnInMainWindow).to_i()) {
      UIActionAdd(this->_plugins_group, ui::powerInMainWnd, 0, "W��cz tryb away", ico::enable);
    }
    if (Config::get(cfg::btnInCntWindow).to_i()) {
      UIGroupAdd(IMIG_MSGTB, ui::msgTbGrp, ACTR_INIT, "kAway2", ico::logoSmall);
      UIActionAdd(ui::msgTbGrp, ui::powerInCntWnd, ACTSC_BOLD, "W��cz tryb away", ico::enable);
      UIActionAdd(ui::msgTbGrp, ui::ignoreBtn, ACTS_DISABLED, "W��cz ignorowanie", ico::ignore);
    }
    if (Config::get(cfg::btnInTrayMenu).to_i()) {
      UIActionInsert(IMIG_TRAY, 0, 0, ACTT_SEP);
      UIActionInsert(IMIG_TRAY, ui::powerInTrayMenu, 0, 0, "W��cz tryb away", ico::enable);
    }
    ev.setSuccess();
  }

  void Controller::onEnd(IMEvent& ev) {
    if (this->isEnabled()) {
      this->disable("", true);
    }
    this->extAutoAwayTimer.reset();
    ev.setSuccess();
  }

  void Controller::onMsgRcv(IMEvent& ev) {
    cMessage* m = (cMessage*) ev.getP1();

    // hmmm, i have to remove it some sunny day...
    if ((m->type != MT_MESSAGE) || (m->flag & MF_AUTOMATED)) {
      return ev.setReturnValue(0);
    }

    // we're searchin' for contact id
    int cnt = Ctrl->ICMessage(IMC_CNT_FIND, m->net, (int) (strlen(m->fromUid) ? m->fromUid : m->toUid));
    bool fromUser = !strlen(m->fromUid) && !GetExtParam(m->ext, cfg::extParamName).length() && (m->type == MT_MESSAGE);

    // parse and return message in case irc-like commands
    if (fromUser) {
      int retValue = _parseMsg(m);
      if (retValue) {
        return ev.setReturnValue(retValue);
      }
    }

    if (this->isEnabled() && !this->cntProp(cnt)->ignored) {
      if (fromUser && ItemFallback(cfg::disableOnMsgSend, cnt).to_b()) {
        this->disable(); return ev.setReturnValue(0);
      }

      if (ItemFallback(cfg::saveToHistory, cnt).to_b()) {
        Helpers::addItemToHistory(m, cnt, cfg::historyFolder, "", this->cntProp(cnt)->historySess);
        this->cntProp(cnt)->historySess = 1;
      }

      // this->addMsg2CntQueue(cnt, m);
      // fCtrl->onNewMsg(cnt, m);

      if (ItemFallback(cfg::reply::onMsg, cnt).to_b()) {
        int intType = ItemFallback(cfg::reply::minIntervalType, cnt).to_i();

        if (strlen(m->fromUid)) {
          int lastMsgTime = this->cntProp(cnt)->lastMsgTime;
          int interval = ItemFallback(cfg::reply::minInterval, cnt).to_i();

          if ((!interval && !lastMsgTime) || (interval && ((interval + lastMsgTime) < m->time))) {
            this->sendMsgTpl(cnt, Controller::tplReply);
          }
          if ((intType == typeRcvTime) || (intType == typeBoth)) {
            this->cntProp(cnt)->lastMsgTime = m->time;
          }
        } else {
          if ((intType == typeSendTime) || (intType == typeBoth)) {
            this->cntProp(cnt)->lastMsgTime = m->time;
          }
        }
      }
    }
  }

  void Controller::_handleCntGroup(ActionEvent& ev) {
    if (ev.isCode(ACTN_CREATE)) {
      UIActionSetStatus(ev.getAction(), !Ctrl->DTgetPos(DTCNT, ev.getCnt()) ? -1 : 0, ACTS_HIDDEN);
    }
  }

  void Controller::_handleMsgTb(ActionEvent& ev) {
    if (ev.isCode(ACTN_CREATEGROUP)) {
      bool isIgnored = this->cntProp(ev.getCnt())->ignored;

      Helpers::chgBtn(ui::msgTbGrp, ui::ignoreBtn, AC_CURRENT, 
        isIgnored ? "Wy��cz ignorowanie" : "W��cz ignorowanie", isIgnored ? ico::unIgnore : ico::ignore,
        isEnabled() ? 0 : ACTS_DISABLED);
      Helpers::chgBtn(ui::msgTbGrp, ui::powerInCntWnd, AC_CURRENT, 
        isEnabled() ? "Wy��cz tryb away" : "W��cz tryb away", isEnabled() ? ico::disable : ico::enable);
    }
  }

  void Controller::_handlePowerBtns(ActionEvent& ev) {
    if (ev.getCode() != ACTN_ACTION) return;

    if (isEnabled()) {
      if (!Config::get(cfg::confirmation).to_i() || Ctrl->ICMessage(IMI_CONFIRM, (int) "Na pewno chcesz wy��czy� tryb away?")) {
        this->disable();
      }
    } else {
      wnd->show();
    }
  }

  void Controller::_handleIgnoreBtn(ActionEvent& ev) {
    if (ev.isCode(ACTN_ACTION) && this->isEnabled()) {
      this->cntProp(ev.getCnt())->ignored = !this->cntProp(ev.getCnt())->ignored;
    }
  }

  void Controller::_clearMRU(ActionEvent& ev) {
    if (ev.isCode(ACTN_ACTION)) {
      mruList->clear();
    }
  }

  void Controller::_resetGlobalSettings(ActionEvent& ev) {
    if (ev.isCode(ACTN_ACTION)) {
      getConfig().resetColumns(tableConfig);

      autoReplyList->load();
      statusList->load();
    }
  }

  void Controller::_resetContactSettings(ActionEvent& ev) {
    if (ev.isCode(ACTN_ACTION)) {
      getConfig().resetColumns(tableContacts);
    }
  }

  void Controller::onPluginsLoaded(IMEvent& ev) {
    if (int oldId = Helpers::pluginExists(plugsNET::kaway)) {
      Ctrl->IMessage(&sIMessage_plugOut(oldId, "kAway2 jest nast�pc� wtyczki K.Away :)",
        sIMessage_plugOut::erNo, sIMessage_plugOut::euNowAndOnNextStart));
      return ev.setFailure();
    }
    if (int ggCrypt = Helpers::pluginExists(plugsNET::ggcrypt)) {
      Ctrl->IMessage(&sIMessage_plugOut(ggCrypt, "Wtyczka GG Crypt jest przestarza�a, przy czym\n"
        "nie pozwala na poprawne dzia�anie wtyczki kAway2.",
        sIMessage_plugOut::erNo, sIMessage_plugOut::euNowAndOnNextStart));
      return ev.setFailure();
    }
    ev.setSuccess();
  }

  void Controller::onExtAutoAway() {
    this->setAutoAway(typeExtended);

    if (Config::get(cfg::autoAwaySync).to_i() == syncExtended && !this->isEnabled()) {
      this->enable(Config::get(cfg::autoAwayMsg), Config::get(cfg::extAutoAway::status).to_i(), true);
    } else {
      this->changeStatus(Config::get(cfg::extAutoAway::status).to_i(), true);
    }
    Ctrl->IMessage(im::extendedAutoAway, NET_BROADCAST);
  }

  void Controller::onAutoAway(IMEvent& ev) {
    if (this->isEnabled()) return;

    int status = Config::get(cfg::status::onAutoAwaySt).to_i();
    int syncType = Config::get(cfg::autoAwaySync).to_i();

    this->setAutoAway(typeBasic);
    this->awayTime.now();

    if (syncType == syncExtended || Config::get(cfg::extAutoAway::status).to_i() != -1) {
      this->extAutoAwayTimer->start((Config::get(cfg::extAutoAway::time).to_i() - Config::get(CFG_AUTOAWAY).to_i()) * 1000);
    }
    if (syncType == syncBasic) {
      this->enable(Config::get(cfg::autoAwayMsg), status, true);
    } else {
      statusCtrl->stringFormatter->addVar("msg", Config::get(cfg::autoAwayMsg));
      statusCtrl->rememberInfo();
      this->changeStatus(status, true);
    }
  }

  void Controller::onBack(IMEvent& ev) {
    if (!this->isAutoAway()) return;

    if (this->isEnabled()) {
      this->extAutoAwayTimer->stop();
      this->disable("", true);
    } else {
      statusCtrl->stringFormatter->clearVars();
      statusCtrl->restoreInfo();
    }

    this->extAutoAwayTimer->stop();
    this->setAutoAway(typeDisabled);
  }

  /* API callback methods */

  void Controller::apiEnabled(IMEvent& ev) {
    ev.setReturnValue(this->isEnabled());
  }

  void Controller::apiEnable(IMEvent& ev) {
    sIMessage* msg = ev.getIMessage();

    logDebug("Remote API Call [enable]: from = %s, msg = %s, status = %i", 
      Helpers::getPlugName(msg->sender), nullChk((char*) msg->p1), msg->p2);
    ev.setReturnValue(this->enable((char*) msg->p1, msg->p2));
  }

  void Controller::apiDisable(IMEvent& ev) {
    sIMessage* msg = ev.getIMessage();

    logDebug("Remote API Call [disable]: from = %s, msg = %s", 
      Helpers::getPlugName(msg->sender), nullChk((char*) msg->p1));
    ev.setReturnValue(this->disable((char*) msg->p1));
  }

  void Controller::apiIgnored(IMEvent& ev) {
    ev.setReturnValue(this->cntProp(ev.getP1())->ignored);
  }

  void Controller::apiAutoAway(IMEvent& ev) {
    ev.setReturnValue(this->isAutoAway());
  }

  void Controller::apiIgnore(IMEvent& ev) {
    sIMessage* msg = ev.getIMessage();

    logDebug("Remote API Call [ignore]: from = %s, cnt = %i, ignore = %s", 
      Helpers::getPlugName(msg->sender), msg->p1, btoa((bool) msg->p2));
    if (this->isEnabled()) {
      this->cntProp(msg->p1)->ignored = (bool) msg->p2;
    }
  }

  void Controller::apiShowAwayWnd(IMEvent& ev) {
    logDebug("Remote API Call [showAwayWnd]: from = %s",
      Helpers::getPlugName(ev.getIMessage()->sender));
    wnd->show();
  }

  /* strictly Controller methods */

  bool Controller::enable(const StringRef& msg, int status, bool silent) {
    if (this->_active) return false;

    if (!this->isAutoAway()) {
      this->awayTime.now();
    }
    this->awayMsg = msg;
    this->_active = true;

    if (this->autoAway) {
      // this->awayTime = this->awayTime.getTime64() - Config::get(CFG_AUTOAWAY).to_i();
    }

    bool chgStatus = Config::get(this->isFromWnd ? cfg::wnd::changeOnEnable : cfg::status::changeOnEnable).to_i();
    bool chgInfo = Config::get(this->isFromWnd ? cfg::wnd::changeInfoOnEnable : cfg::status::changeInfoOnEnable).to_i();

    if (chgInfo) {
      statusCtrl->stringFormatter->addVar("msg", msg);
    }

    if (chgStatus || chgInfo) {
      int defStatus = Config::get(this->isFromWnd ? cfg::wnd::onEnableSt : cfg::status::onEnableSt).to_i();
      status = !chgStatus ? (this->autoAway ? status : -1) : (status ? status : defStatus);

      if (!statusCtrl->isRemembered()) {
        statusCtrl->rememberInfo();
      }
      this->changeStatus(status, chgInfo);
    }

    if (Config::get(this->isFromWnd ? cfg::wnd::muteOnEnable : cfg::muteOnEnable).to_i() && !Config::get(kSound::Cfg::mute).to_i()) {
      Helpers::UIActionCall(this->_plugins_group, kSound::action::mute);
      this->muteStateSwitched = true;
    }

    this->switchBtns(true);
    this->fromWnd(false);

    int count = Ctrl->IMessage(IMC_CNT_COUNT);
    for (int i = 1; i < count; i++) {
      if (Helpers::isMsgWndOpen(i)) {
        if (ItemFallback(cfg::reply::onEnable, i).to_b() && !silent && !this->cntProp(i)->ignored) {
          this->sendMsgTpl(i, tplEnable);
        }
      }
    }

    this->showKNotify("Tryb away zosta� <b>w��czony<b>", ico::enable);
    log("[Controller::enable()]: msg = %s, silent = %s", nullChk(msg), btoa(silent));
    Ctrl->IMessage(im::away, NET_BROADCAST, -1, (int) msg.c_str(), status);

    return true;
  }

  bool Controller::disable(const StringRef& msg, bool silent) {
    if (!this->_active) return false;

    statusCtrl->stringFormatter->clearVars();
    statusCtrl->restoreInfo();

    if (this->muteStateSwitched && Config::get(kSound::Cfg::mute).to_i()) {
      Helpers::UIActionCall(this->_plugins_group, kSound::action::mute);
    }

    this->switchBtns(false);

    int count = Ctrl->IMessage(IMC_CNT_COUNT);
    for (int i = 1; i < count; i++) {
      if (Helpers::isMsgWndOpen(i)) {
        if (ItemFallback(cfg::reply::onDisable, i).to_b() && !silent && !this->cntProp(i)->ignored) {
          this->sendMsgTpl(i, tplDisable, msg);
        }
      }
    }

    for (tCnts::iterator it = this->cntProps.begin(); it != this->cntProps.end(); it++) {
      logDebug("[Controller::disable().saved]: cnt = %i, ignored = %s", it->first, btoa(it->second.ignored));
      for (tMsgQueue::iterator it2 = it->second.msgQueue.begin(); it2 != it->second.msgQueue.end(); it2++) {
        logDebug("[Controller::disable().saved.item]: msg = %s", (*it2)->body);
        messageFree((*it2), true);
      }
    }

    this->awayMsg = "";
    this->awayTime.clear();
    this->cntProps.clear();

    this->_active = false;
    this->muteStateSwitched = false;

    this->showKNotify("Tryb away zosta� <b>wy��czony<b>", ico::disable);
    log("[Controller::disable()]: msg = %s, silent = %s", nullChk(msg), btoa(silent));
    Ctrl->IMessage(im::back, NET_BROADCAST, -1, (int) msg.c_str());

    return true;
  }

  void Controller::showKNotify(const char * text, int ico) {
    if (!Config::get(cfg::useKNotify).to_i() || !Helpers::pluginExists(plugsNET::knotify)) return;
    Helpers::showKNotify((char*) text, ico);
  }

  void Controller::switchBtns(bool state) {
    String name = (state) ? "Wy��cz tryb away" : "W��cz tryb away";
    int ico = (state) ? ico::disable : ico::enable;

    Helpers::chgBtn(IMIG_TRAY, ui::powerInTrayMenu, name.a_str(), ico);
    Helpers::chgBtn(this->_plugins_group, ui::powerInMainWnd, name.a_str(), ico, 
      (state && (Helpers::findParentAction(IMIG_MAINWND, this->_plugins_group) != IMIG_MAINTB)) ? ACTS_CHECKED : 0);
    Helpers::chgBtn(IMIG_MSGTB, ui::msgTbGrp, "kAway2", ico::logoSmall, ACTR_INIT | ACTS_GROUP | (state ? ACTS_CHECKED : 0));

    int count = Ctrl->IMessage(IMC_CNT_COUNT);
    for (int i = 1; i < count; i++) {
      if (!Helpers::isMsgWndOpen(i)) {
        continue;
      }
      Helpers::chgBtn(IMIG_MSGTB, ui::msgTbGrp, i, "kAway2", ico::logoSmall, ACTR_INIT | ACTS_GROUP | (state ? ACTS_CHECKED : 0));
    }
  }

  void Controller::changeStatus(int _status, bool changeInfo) {
    int actSt, status, chgOnlyIfOnline = (this->autoAway == typeBasic) && Config::get(cfg::status::chgOnlyIfOnline).to_i();
    NetList::tItems& nets = statusList->getItems();

    if (changeInfo) {
      statusCtrl->stringFormatter->addVar("date", this->awayTime.strftime(Config::get(cfg::dateFormat).to_s().a_str()));
      statusCtrl->stringFormatter->addVar("time", this->awayTime.strftime(Config::get(cfg::timeFormat).to_s().a_str()));
    }

    for (NetList::tItems::iterator it = nets.begin(); it != nets.end(); it++) {
      actSt = statusCtrl->getActualStatus(it->getNet());
      status = _status;

      if (!statusCtrl->isNetValid(it->getNet())) {
        continue;
      }
      if (chgOnlyIfOnline && (actSt != ST_ONLINE && actSt != ST_CHAT)) {
        status = -1;
      }
      if (changeInfo) {
        statusCtrl->changeStatusInfo(it->getNet(), Config::get(cfg::tpl::status), status);
      } else if (status != -1) {
        statusCtrl->changeStatus(it->getNet(), status);
      }
    }
  }

  int Controller::_parseMsg(cMessage* m) {
    int cnt = Ctrl->ICMessage(IMC_CNT_FIND, m->net, (int) (strlen(m->fromUid) ? m->fromUid : m->toUid));

    if (strlen(m->body) <= 1 || m->body[0] != '/' || !ItemFallback(cfg::ircCmds, cnt).to_b()) {
      return 0;
    }

    tStringVector params;
    std::string body(m->body); 
    body = body.substr(1, body.length());

    Stamina::split(body, " ", params);
    String cmd = params[0];
    String awayMsg = (params.size() > 1) ? body.substr(params[0].length() + 1, body.length()) : "";

    bool del = false, set = false; // irc-type command like
    int st = 0; // default status

    RegEx reg;
    reg.match("/^(away|brb)\\[(.+)\\]$/", cmd.a_str());

    if (reg.isMatched()) {
      cmd = reg[1];

      if      (reg[2] == "away")    st = ST_AWAY;
      else if (reg[2] == "xa")      st = ST_NA;
      else if (reg[2] == "dnd")     st = ST_DND;
      else if (reg[2] == "hidden")  st = ST_HIDDEN;
      else if (reg[2] == "-")       st = -1;
    }

    if (cmd == "away") {
      del = true; set = this->enable(awayMsg, st);
    } else if (cmd == "brb") {
      del = true; set = this->enable(awayMsg, st, true);
    } else if (cmd == "back") {
      del = true; set = this->disable(awayMsg);
    } else if (cmd == "re") {
      del = true; set = this->disable(awayMsg, true);
    }

    if (del) {
      logDebug("{IM_MSG_RCV}: cmd = %s, reason = %s, params = %i",
        cmd.c_str(), nullChk(awayMsg), params.size());

      if (!set) {
        return IM_MSG_delete;
      }

      String body, ext(m->ext);

      body = stringf("<u>%s</u> tryb away", (cmd == "away" || cmd == "brb") ? "W��czono" : "Wy��czono");
      if (!awayMsg.empty()) {
        body += stringf(" (<b>%s</b>)", awayMsg.c_str());
      }

      ext = SetExtParam(ext, MEX_ADDINFO, "kAway2");
      ext = SetExtParam(ext, MEX_NOSOUND, "1");

      char * newBody = (char*) Ctrl->GetTempBuffer(body.length() + 1 + ext.length() + 1);
      strcpy(newBody, body.a_str());
      char * newExt = newBody + body.length() + 2;
      strcpy(newExt, ext.a_str());

      m->body = newBody;
      m->ext = newExt;
      m->flag |= (m->flag & MF_HTML) ? 0 : MF_HTML;
      m->flag |= MF_DONTADDTOHISTORY;

      return IM_MSG_update | IM_MSG_delete;
    }
    return 0;
  }

  void Controller::sendMsgTpl(int cnt, enAutoMsgTpl tpl, const StringRef& msgVal) {
    int session, net = Config::get(CNT_NET, cnt).to_i();

    if (((statusCtrl->getActualStatus(net) == ST_HIDDEN) && !ItemFallback(cfg::reply::whenInvisible, cnt).to_b()) || 
      !autoReplyList->getItem(net).isActive() || !autoReplyList->getItem(net).isConnected())
      return;

    String ext, uid(Config::get(CNT_UID, cnt));
    ext = SetExtParam(ext, cfg::extParamName, inttostr(tpl));
    ext = SetExtParam(ext, MEX_ADDINFO, "kAway2");
    ext = SetExtParam(ext, MEX_NOSOUND, "1");

    Format format;
    format.addVar("uid", uid);
    format.addVar("display", Config::get(CNT_DISPLAY, cnt));
    format.addVar("name", Config::get(CNT_NAME, cnt));
    format.addVar("nick", Config::get(CNT_NICK, cnt));
    format.addVar("surname", Config::get(CNT_SURNAME, cnt));
    format.addVar("date", Helpers::isToday(this->awayTime) ? "" : this->awayTime.strftime(Config::get(cfg::dateFormat).to_s().a_str()));
    format.addVar("time", this->awayTime.strftime(Config::get(cfg::timeFormat).to_s().a_str()));
    format.addVar("msg", (tpl == tplDisable) ? msgVal : this->awayMsg);

    /*
    String body = Helpers::trim(format.parse(ItemFallback(tpl, cnt).to_s()));
    cMessage msg = Message::prepare(uid, "", net, body, MT_MESSAGE, ext, 
      MF_SEND | (ItemFallback(cfg::reply::useHtml, cnt).to_b() ? MF_HTML : 0));

    HWND hwndMsg = (HWND) UIGroupHandle(sUIAction(0, IMIG_MSGWND, cnt));
    if (!(session = (int) GetProp(hwndMsg, "MsgSession"))) {
      SetProp(hwndMsg, "MsgSession", (HANDLE) 1);
    }

    Helpers::addItemToHistory(&msg, cnt, "messages", "", session);
    Message::send(&msg);

    if (ItemFallback(cfg::reply::showInWnd, cnt).to_b()) {
      Message::inject(&msg, cnt);
    }
    */
    Message::send(cnt, Helpers::trim(format.parse(ItemFallback(tpl, cnt).to_s())), 
      MT_MESSAGE, ext, ItemFallback(cfg::reply::useHtml, cnt).to_b());

    logDebug("[Controller::sendMsgTpl()]: msg.tpl = %i", tpl);
  }
}