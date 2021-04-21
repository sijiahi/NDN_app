#ifndef COMMON_H
#define COMMON_H

#include <cstddef>
#include <list>
#include <set>
#include <map>
#include <queue>
#include <vector>
#include <string>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/data.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>

namespace chronochat {

using std::size_t;

using boost::noncopyable;

using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::enable_shared_from_this;
using std::make_shared;
using std::static_pointer_cast;
using std::dynamic_pointer_cast;
using std::const_pointer_cast;
using std::function;
using std::bind;
using std::ref;
using std::cref;

using ndn::Interest;
using ndn::Data;
using ndn::Name;
//using ndn::Exclude;
using ndn::Block;
using ndn::Signature;
using ndn::KeyLocator;

namespace tlv {
using namespace ndn::tlv;
}

namespace name = ndn::name;
namespace time = ndn::time;

} // namespace chronochat
namespace repo{
static const uint64_t DEFAULT_BLOCK_SIZE = 1000;
static const uint64_t DEFAULT_INTEREST_LIFETIME = 4000;
static const uint64_t DEFAULT_FRESHNESS_PERIOD = 10000;
static const uint64_t DEFAULT_CHECK_PERIOD = 10000;
static const size_t PRE_SIGN_DATA_COUNT = 10;
}
#endif // COMMON_H
