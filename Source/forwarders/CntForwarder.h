/**
 *  Contact Forwarder class
 *
 *  Licensed under The GNU Lesser General Public License
 *  Redistributions of files must retain the above copyright notice.
 *
 *  @filesource
 *  @copyright    Copyright (c) 2005-2006 Sijawusz Pur Rahnama
 *  @link         svn://kplugins.net/kaway2/ kAway2 plugin SVN Repo
 *  @version      $Revision$
 *  @modifiedby   $LastChangedBy$
 *  @lastmodified $Date$
 *  @license      http://creativecommons.org/licenses/LGPL/2.1/
 */

#pragma once

#include "../stdafx.h"
#include "../main.h"

#include "../Helpers.h"
#include "../Control.h"
#include "../Format.h"

namespace kAway2 {
  namespace ui {
    namespace forward {
      const unsigned int forward = ui + 40;

      const unsigned int cfgGroup = forward + 1;
      const unsigned int userCombo = forward + 2;
    }
  }

  namespace ico {
    const unsigned int forward = ico + 14;
  }

  namespace cfg {
    namespace forward {
      const unsigned int forward = cfg + 500;

      const unsigned int cnt = forward + 1;

      const unsigned int isSummaryActive = forward + 2;
      const unsigned int isForwardActive = forward + 3;
    }

    namespace tpl {
      const unsigned int forward = tpl + 4;
      const unsigned int forwardSummary = tpl + 9;
    }
  }

  class CntForwarder : public Forwarder {
  public:
    CntForwarder();
    ~CntForwarder();

  public:
    inline bool preSummary() {
      return(GETINT(cfg::forward::cnt));
    }

    inline bool preForward(int cnt, cMessage *msg) {
      return(this->preSummary());
    }

    inline void setCfgCols() {
      this->cfgCols["tplSummary"] = cfg::tpl::forwardSummary;
      this->cfgCols["tplForward"] = cfg::tpl::forward;
      this->cfgCols["cfgGroup"] = ui::forward::cfgGroup;

      this->cfgCols["isSummaryActive"] = cfg::forward::isSummaryActive;
      this->cfgCols["isForwardActive"] = cfg::forward::isForwardActive;
    }

    void send(std::string msg);
    void onISetCols();
    void onIPrepare();
    void onAction(int id, int code);
  };
}