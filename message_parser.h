#ifndef MESSAGE_PARSER_H
#define MESSAGE_PARSER_H
#include "common.h"
#include <ndn-cxx/util/concepts.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <boost/concept_check.hpp>
using ndn::Block;
using ndn::EncodingEstimator;
class message_parser
{
public:

  class Error : public std::runtime_error
  {
  public:
    explicit
    Error(const std::string& what)
      : std::runtime_error(what)
    {
    }
  };
public:
    message_parser();

    explicit
    message_parser(const Block& chatMsgWire);

    const Block&
    wireEncode() const;

    void
    wireDecode(const Block& chatMsgWire);

    const std::string&
    getData();


private:
  mutable Block m_wire;
  std::string m_data;
};


#endif // MESSAGE_PARSER_H
