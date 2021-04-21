#include "message_parser.h"
#include <iostream>

message_parser::message_parser()
{

}
message_parser::message_parser(const Block& rawMsg)
{
    EncodingEstimator estimater;
  this->wireDecode(rawMsg);
}



const Block&
message_parser::wireEncode() const{
    return m_wire;
}

void
message_parser::wireDecode(const Block& chatMsgWire){
    m_wire=chatMsgWire;
    m_data=std::string(reinterpret_cast<const char* >(m_wire.value()),
                           m_wire.value_size());
    std::cout<<m_data<<std::endl;
}

const std::string&
message_parser::getData(){
    return m_data;
}
