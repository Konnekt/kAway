/**
  *  Forwarders Controller class
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

#pragma once

#ifndef __FWDCONTROLLER_H__
#define __FWDCONTROLLER_H__

#include "Helpers.h"
#include "Forwarder.h"

namespace kAway2 {
  class FwdController : public SharedObject<iSharedObject> {
  public:
    typedef std::deque<Forwarder*> tForwarders;

  protected:
    int summarizableCount;
    int forwardableCount;

  public:
    FwdController();
    ~FwdController();

    void UIDrawActiveSum();
    void UIDrawActiveFwd();

    void registerCfgGroups(IMEvent &ev);
    Forwarder* getById(const StringRef& id);
    void fwdRegister(Forwarder *f);

  public:
    tForwarders forwarders;
  };

  typedef SharedPtr<FwdController> oFwdController;
}

#endif // __FWDCONTROLLER_H__