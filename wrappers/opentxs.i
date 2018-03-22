/*

  Main file for the opentxs swig wrappers to define any variables
  common to all wrappers. This is included from each wrapper's OTX_.i

  This was created from the opentsx.i which once contained all wrapper
  directives. Each wrapper has been migrated to its own sub-directory.

*/

// add the following to every .cxx file.
%inline %{
#ifndef SWIG_VERSION
#define SWIG_VERSION
#endif

#include "opentxs/client/NymData.hpp"
#include "opentxs/client/OTRecordList.hpp"
#include "opentxs/client/SwigWrap.hpp"
#include "opentxs/core/crypto/OTCallback.hpp"
#include "opentxs/core/crypto/OTCaller.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PairSocket.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/network/zeromq/PullSocket.hpp"
#include "opentxs/network/zeromq/PushSocket.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"
#include "opentxs/network/zeromq/RequestSocket.hpp"
#include "opentxs/network/zeromq/Socket.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivitySummaryItem.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/ContactListItem.hpp"
#include "opentxs/ui/ListRow.hpp"
#include "opentxs/ui/MessagableList.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
%}

%apply std::string &OUTPUT { std::string& STR_RETAINED_COPY };

%feature("director") OTCallback;
%feature("director") OTNameLookup;
%feature("director") network::zeromq::ListenCallback;
%feature("director") network::zeromq::ReplyCallback;

/* Parse the header file to generate wrappers */
#ifndef EXPORT
#define EXPORT
#endif

%include "carrays.i"
%include "cdata.i"
%include "cpointer.i";
%include "inttypes.i";
%include "std_pair.i"
%include "std_string.i";
%include "typemaps.i"

namespace std {
  typedef uint64_t size_t;
};

typedef int64_t time64_t;

%include "../../include/opentxs/client/NymData.hpp"
%include "../../include/opentxs/Pimpl.hpp"
%include "../../include/opentxs/core/Data.hpp"
%include "../../include/opentxs/core/crypto/OTPassword.hpp"
%include "../../include/opentxs/core/crypto/OTCallback.hpp"
%include "../../include/opentxs/core/crypto/OTCaller.hpp"
%include "../../include/opentxs/ui/ListRow.hpp"
%include "../../include/opentxs/ui/ActivitySummaryItem.hpp"
%include "../../include/opentxs/ui/ActivitySummary.hpp"
%include "../../include/opentxs/ui/ActivityThreadItem.hpp"
%include "../../include/opentxs/ui/ActivityThread.hpp"
%include "../../include/opentxs/ui/ContactListItem.hpp"
%include "../../include/opentxs/ui/ContactList.hpp"
%include "../../include/opentxs/ui/MessagableList.hpp"
%include "../../include/opentxs/network/zeromq/Message.hpp"
%include "../../include/opentxs/Types.hpp"
%include "../../include/opentxs/network/zeromq/Socket.hpp"
%include "../../include/opentxs/network/zeromq/PublishSocket.hpp"
%include "../../include/opentxs/network/zeromq/ListenCallback.hpp"
%include "../../include/opentxs/network/zeromq/SubscribeSocket.hpp"
%include "../../include/opentxs/network/zeromq/PullSocket.hpp"
%include "../../include/opentxs/network/zeromq/PushSocket.hpp"
%include "../../include/opentxs/network/zeromq/ReplyCallback.hpp"
%include "../../include/opentxs/network/zeromq/ReplySocket.hpp"
%include "../../include/opentxs/network/zeromq/RequestSocket.hpp"
%include "../../include/opentxs/network/zeromq/PairSocket.hpp"
%include "../../include/opentxs/network/zeromq/Context.hpp"
%include "../../include/opentxs/client/SwigWrap.hpp"
%include "../../include/opentxs/client/OTRecordList.hpp"

// add the following to every .cxx file.
%inline %{
  using namespace opentxs;
%}
