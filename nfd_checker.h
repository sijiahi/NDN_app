#ifndef NFD_CHECKER_H
#define NFD_CHECKER_H

#include <ndn-cxx/data.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <fstream>
#include <iostream>
#include <unistd.h>

#include "messager.h"
#include <boost/lexical_cast.hpp>

using ndn::Name;
using ndn::Interest;
using ndn::Data;
using ndn::lp::Nack;
using ndn::Face;
class nfd_checker
{
public:
    nfd_checker();
    bool
    check();
private:

    void
    onData(const Interest& interest, const Data& data);
    void
    onNack(const Interest& interest, const Nack& nack);
    void
    onTimeout(const Interest& interest);
private:
  Face m_face;
};

#endif // NFD_CHECKER_H
