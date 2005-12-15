/*
 *  kAway2 Helpers
 *
 *  Please READ /License.txt FIRST!
 *
 *  Copyright (C)2005 Sijawusz Pur Rahnama
 *
 *  $Id$
 */

#pragma once

std::string itos(int i, int radix = 10) {
  char buff[64]; _itoa(i, buff, radix);
  return(buff);
}

char * btoa(bool value) {
  return(value ? "yes" : "no");
}

std::string btos(bool value) {
  return(btoa(value));
}

namespace kAway2 {
  namespace Helpers {
    std::string icon32(int ico) {
      std::string buff = AP_IMGURL + std::string("reg://IML32/") + itos(ico) + ".ico";
      return(buff);
    }

    std::string icon16(int ico) {
      std::string buff = "reg://IML16/" + itos(ico) + ".ico";
      return(buff);
    }

    std::string trim(std::string txt) {
      CStdString buff(txt); txt = buff.Trim();
      return(txt);
    }

    void UIActionCall(int group, int act, int cntID = 0) {
      UIActionCall(&sUIActionNotify_2params(sUIAction(group, act, cntID), ACTN_ACTION, 0, 0));
    }

    void showKNotify(char * text, int ico) {
      if (!GETINT(cfg::useKNotify)) return;
      Ctrl->IMessage(&KNotify::sIMessage_notify(text, ico));
    }

    void clearMru(const char * name) {
      sMRU mru;

      mru.name = name;
      mru.count = 0;
      mru.flags = MRU_GET_USETEMP;

      Ctrl->IMessage(&sIMessage_MRU(IMC_MRU_SET, &mru));
    }
  }
}