/*

  Main file for the opentxs swig wrappers to define any variables
  common to all wrappers. This is included from each wrapper's OTX_.i

  This was created from the opentsx.i which once contained all wrapper
  directives. Each wrapper has been migrated to its own sub-directory.

*/


%ignore strtoimax;
%ignore strtoumax;

%include "inttypes.i"
%include "std_string.i";
%include "std_vector.i";
%include "std_map.i"
%include "typemaps.i"
%include <std_shared_ptr.i>

typedef int64_t time64_t;

%{
#ifndef IMPORT
#define IMPORT
#endif

#include "opentxs/client/NymData.hpp"
#include "opentxs/client/OT_ME.hpp"
#include "opentxs/client/OTRecord.hpp"
#include "opentxs/client/OTRecordList.hpp"
#include "opentxs/client/SwigWrap.hpp"
#include "opentxs/core/crypto/OTCallback.hpp"
#include "opentxs/core/crypto/OTCaller.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"
#include "opentxs/network/zeromq/RequestSocket.hpp"
#include "opentxs/network/zeromq/Socket.hpp"
#include "opentxs/Types.hpp"

#include <string>
#include <vector>
#include <map>

#ifdef ANDROID
#ifndef imaxdiv
imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom) {
    imaxdiv_t res;
    res.quot=0; res.rem=0;
    while(numer >= denom) {
        res.quot++;
        numer -= denom;
    }
    res.rem = numer;
    return res;
}
#endif

#ifndef imaxabs
intmax_t imaxabs(intmax_t j) {
    return (j < 0 ? -j : j);
}
#endif
#endif

%}

namespace std {
   %template(VectorUnsignedChar) vector<unsigned char>;
   %template(MapStringString) map<string,string>;
};

%ignore OTRecord::operator<(const OTRecord & rhs);
%ignore OTPassword::operator=(const OTPassword & rhs);
%ignore OTPasswordData;
%ignore clone;
%ignore Storable::Create(StoredObjectType eType, PackType thePackType);
%ignore OTPasswordData;
%ignore PackedBuffer;
%ignore OTPacker;
%ignore PackerSubclass;
%ignore Storage::Create(StorageType eStorageType, PackType ePackType);
%ignore stat;
%ignore std::istream;
%ignore std::ostream;
%ignore msgpack::sbuffer;
%ignore std::map<std::string, std::string>;
%ignore stlplus::simple_ptr_clone;


%ignore weak_ptr_OTRecord;

%ignore vec_OTRecordList;
%ignore list_of_strings;
%ignore map_of_strings;


%rename(OTRecordLessThan) opentxs::OTRecord::operator<(const OTRecord& rhs);
%apply std::string &OUTPUT { std::string& STR_RETAINED_COPY };

%feature("director") OTCallback;
%feature("director") OTNameLookup;

/* Parse the header file to generate wrappers */

#ifndef EXPORT
#define EXPORT
#endif

%include "../../include/opentxs/network/zeromq/Message.hpp"
%include "../../include/opentxs/network/zeromq/Socket.hpp"
%include "../../include/opentxs/network/zeromq/ReplySocket.hpp"
%include "../../include/opentxs/network/zeromq/RequestSocket.hpp"
%include "../../include/opentxs/network/zeromq/Context.hpp"
%include "../../include/opentxs/client/NymData.hpp"
%include "../../include/opentxs/client/OTRecord.hpp"
%include "../../include/opentxs/client/OTRecordList.hpp"
%include "../../include/opentxs/client/SwigWrap.hpp"
%include "../../include/opentxs/core/crypto/OTCallback.hpp"
%include "../../include/opentxs/core/crypto/OTCaller.hpp"
%include "../../include/opentxs/core/crypto/OTPassword.hpp"

bool opentxs::OT_API_Set_PasswordCallback(OTCaller & theCaller);
bool opentxs::OT_API_Set_AddrBookCallback(OTLookupCaller & theCaller);


// add the following to every .cxx file.
%inline %{
  using namespace opentxs;
%}
