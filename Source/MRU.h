/**
  *  MRU class
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

#ifndef __MRU_H__
#define __MRU_H__

class MRU : public SharedObject<iSharedObject> {
public:
  typedef std::deque<String> tItems;

  /*
   * Class version
   */
	STAMINA_OBJECT_CLASS_VERSION(MRU, iSharedObject, Version(0,1,0,0));

public:
  MRU(const string& name, int count = 100) : _name(name), _count(count) { }

public:
  tItems get(bool rev = true, const char * buff = 0, int buffSize = 1024);

  void append(const StringRef& current);
  void append(const tItems& list);

  void set(const StringRef& current);
  void set(const tItems& list);

  void clear();

  virtual inline int getCount() {
    return _count;
  }
  virtual inline void setCount(int count) {
    _count = count;
  }

protected:
  void _append(const StringRef& current, int count);

protected:
  string _name;
  unsigned int _count;
};

typedef SharedPtr<MRU> oMRU;

#endif // __MRU_H__