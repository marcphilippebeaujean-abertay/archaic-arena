#include "message.h"



message::message()
{
	// initialise an empty packet
	initPacketValues(0, sf::Vector2f(0, 0), 0, 0, 0.0f, RECV, 0.0f);
}


message::~message()
{
}

void message::initPacketValues(sf::Int16 cmdID, sf::Vector2f msgVec, sf::Int64 msgValue, sf::Int16 clientID, float latencyTime, MESSAGE_TYPE msgType, float sentTime)
{
	// populate the message based on value
	commandID = cmdID;
	this->msgVec = msgVec;
	this->msgValue = msgValue;
	this->clientID = clientID;
	this->latencyTime = latencyTime;
	this->msgType = msgType;
	this->sendTime = sentTime;
}

void message::unwrapPacket(sf::Packet packet)
{
	// unwrap the packet
	packet >> commandID >> msgVec.x >> msgVec.y >> msgValue >> clientID >> latencyTime >> sendTime;
	msgType = RECV;
}

sf::Packet message::createPacket()
{
	// populate the packet with information
	sf::Packet packet;
	packet << commandID << msgVec.x << msgVec.y << msgValue << clientID << latencyTime << sendTime;
	return packet;
}