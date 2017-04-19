#ifndef PSTATE_H
#define PSTATE_H

#define USER_DEFINED_PSTATE_COMP

#include <string>
#include <set>
#include <map>
#include <utility>
#include "Labeler.h"
#include "AbstractValue.h"
#include "VariableIdMapping.h"
#include "ConstraintRepresentation.h"

typedef int PStateId;

#include "HashFun.h"
#include "HSetMaintainer.h"

using CodeThorn::ConstraintSet;
using CodeThorn::ConstraintSetMaintainer;
using SPRAY::Edge;

//using namespace SPRAY;

namespace CodeThorn {

  class VariableValueMonitor;
  class Analyzer;
/*! 
  * \author Markus Schordan
  * \date 2012.
 */
class PState : public map<SPRAY::VariableId,CodeThorn::AValue> {
 public:
    PState() {
    }
  friend std::ostream& operator<<(std::ostream& os, const PState& value);
  friend std::istream& operator>>(std::istream& os, PState& value);
  bool varExists(SPRAY::VariableId varId) const;
  bool varIsConst(SPRAY::VariableId varId) const;
  bool varIsTop(SPRAY::VariableId varId) const;
  CodeThorn::AValue varValue(SPRAY::VariableId varId) const;
  string varValueToString(SPRAY::VariableId varId) const;
  void deleteVar(SPRAY::VariableId varname);
  long memorySize() const;
  void fromStream(std::istream& is);
  void toStream(std::ostream& os) const;
  string toString() const;
  string toString(SPRAY::VariableIdMapping* variableIdMapping) const;
  void setAllVariablesToTop();
  void setAllVariablesToValue(CodeThorn::AValue val);
  void setVariableToTop(SPRAY::VariableId varId);
  void setVariableToValue(SPRAY::VariableId varId, CodeThorn::AValue val);
  void topifyState();
  bool isTopifiedState() const;
  SPRAY::VariableIdSet getVariableIds() const;
  static void setActiveGlobalTopify(bool val);
  static void setVariableValueMonitor(VariableValueMonitor* vvm);
  static bool _activeGlobalTopify;
  static VariableValueMonitor* _variableValueMonitor;
  static Analyzer* _analyzer;
};

  std::ostream& operator<<(std::ostream& os, const PState& value);
  std::istream& operator>>(std::istream& os, PState& value);

  typedef set<const PState*> PStatePtrSet;

#ifdef USE_CUSTOM_HSET
class PStateHashFun {
   public:
    PStateHashFun(long prime=9999991) : tabSize(prime) {}
    long operator()(PState s) const {
      unsigned int hash=1;
      for(PState::iterator i=s.begin();i!=s.end();++i) {
        hash=((hash<<8)+((long)(*i).second.hash()))^hash;
      }
      return long(hash) % tabSize;
    }
      long tableSize() const { return tabSize;}
   private:
    long tabSize;
};
#else
class PStateHashFun {
   public:
    PStateHashFun() {}
    long operator()(PState* s) const {
      unsigned int hash=1;
      for(PState::iterator i=s->begin();i!=s->end();++i) {
        hash=((hash<<8)+((long)(*i).second.hash()))^hash;
      }
      return long(hash);
    }
   private:
};
#endif
class PStateEqualToPred {
   public:
    PStateEqualToPred() {}
    bool operator()(PState* s1, PState* s2) const {
      if(s1->size()!=s2->size()) {
        return false;
      } else {
        for(PState::iterator i1=s1->begin(), i2=s2->begin();i1!=s1->end();(++i1,++i2)) {
          if(*i1!=*i2)
            return false;
        }
      }
      return true;
    }
   private:
};

/*! 
  * \author Markus Schordan
  * \date 2012.
 */
 class PStateSet : public HSetMaintainer<PState,PStateHashFun,PStateEqualToPred> {
 public:
  typedef HSetMaintainer<PState,PStateHashFun,PStateEqualToPred>::ProcessingResult ProcessingResult;
  string toString();
  PStateId pstateId(const PState* pstate);
  PStateId pstateId(const PState pstate);
  string pstateIdString(const PState* pstate);
 private:
};

// define order for PState elements (necessary for PStateSet)
#ifdef  USER_DEFINED_PSTATE_COMP
bool operator<(const PState& c1, const PState& c2);
#if 0
bool operator==(const PState& c1, const PState& c2);
bool operator!=(const PState& c1, const PState& c2);
#endif
#endif

} // namespace CodeThorn

#endif
